/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Mon, 20 Jul 2015 11:22:48 +0800
 */
#include <signal.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "websrv.h"
#include "mongoose.h"
#include "cJSON.h"
#include "auth_misc.h"

#define HTTPD_PASSWORDS_PATH "/ugwconfig/etc/httpd.passwords"
#define AUTH_DOMAIN "AP-admin"

static int s_received_signal = 0;
static struct mg_server *s_server = NULL;
static const char *s_auth_server_addr = "udp://127.0.0.1:2222";
static struct mg_connection *s_auth_server_conn = NULL;

static void signal_handler(int sig_num) {
  signal(sig_num, signal_handler);
  s_received_signal = sig_num;
}

static unsigned long auth_req_id = 0;

static int auth_login_request(struct mg_connection *conn, struct auth_conn_param *acp);
static int auth_online_request(struct mg_connection *conn, struct auth_conn_param *acp);
static int auth_admin_request(struct mg_connection *conn, struct auth_conn_param *acp);
static int auth_admin_reboot(struct mg_connection *conn, struct auth_conn_param *acp);
static int auth_admin_reset(struct mg_connection *conn, struct auth_conn_param *acp);
static int auth_admin_set_config(struct mg_connection *conn, struct auth_conn_param *acp);
static int auth_admin_get_config(struct mg_connection *conn, struct auth_conn_param *acp);
static int modify_passwords_file(const char *fname, const char *domain, const char *user, const char *pass);

static int ev_handler(struct mg_connection *conn, enum mg_event ev) {
  struct auth_conn_param *acp;
  time_t current_time = time(NULL);

  switch (ev) {
    case MG_AUTH:
      printf("conn->uri=%s, %s\n", conn->uri, conn->local_ip);
      if (memcmp(conn->uri, "/admin", 6) == 0 ||
          memcmp(conn->local_ip, "169.254.254.254", 15) == 0)
      {
        int ret = MG_FALSE; // Not authorized
        FILE *fp;
        if ((fp = fopen(HTTPD_PASSWORDS_PATH, "r")) != NULL) {
          ret = mg_authorize_digest(conn, fp);
          fclose(fp);
        }
        return ret;
      }
      return MG_TRUE;

    case MG_CONNECT:
      printf("MG_CONNECT\n");
      return MG_TRUE;

    case MG_CLIENT_UDP_REPLY:
      // Send reply to the original connection
      printf("MG_CLIENT_UDP_REPLY\n");
      if (s_auth_server_conn == conn)
      {
        struct mg_connection *c;
        struct auth_response_data *rsp;

        rsp = (struct auth_response_data *)conn->content;
        for (c = mg_next(s_server, NULL); c != NULL; c = mg_next(s_server, c)) {
          acp = (struct auth_conn_param *)c->connection_param;

          if (!acp)
            continue;
          if (acp->req.id == rsp->id)
          {
            printf("get id=%lu c=%p\n", rsp->id, c);
            if ((rsp->flags & AUTH_USER_LOGIN_SUCCESS))
            {
              mg_printf_data(c, "302 login success");
            }
            else if ((rsp->flags & AUTH_USER_ONLINE_SUCCESS))
            {
              mg_printf_data(c, "302 check online ok");
              printf("302 check online ok\n");
            }
            else
            {
              mg_printf_data(c, "202 failed\n");
            }
            acp->status = WEB_AUTH_DONE;
            break;
          }
        }
      }
      return MG_TRUE;

    case MG_REQUEST:
      printf("MG_REQUEST, conn=%p, param=%p\n", conn, conn->connection_param);
      printf("uri=%s\n", conn->uri);

      if (memcmp(conn->local_ip, "169.254.254.254", 15) == 0 &&
          strcmp(conn->uri, "/") == 0)
      {
        mg_printf(conn,
            "HTTP/1.1 301 Moved Permanently\r\n"
            "Location: /admin/\r\n\r\n");
        return MG_TRUE;
      }

      //assert(!conn->connection_param);
      if (!conn->connection_param)
      {
        conn->connection_param = malloc(sizeof(struct auth_conn_param));
        if (!conn->connection_param)
          return MG_TRUE;
      }

      acp = (struct auth_conn_param *)conn->connection_param;
      memset(acp, 0, sizeof(struct auth_conn_param));
      acp->time = current_time;

      if (memcmp(conn->uri, "/index.html", 11) == 0)
      {
        acp->status = WEB_AUTH_INDEX;
        return MG_FALSE;
      }
      if (memcmp(conn->uri, "/cloudlogin", 11) == 0)
      {
        return auth_login_request(conn, acp);
      }
      if (memcmp(conn->uri, "/cloudonline", 12) == 0)
      {
        return auth_online_request(conn, acp);
      }
      if (memcmp(conn->uri, "/admin/admin.cgi", 16) == 0)
      {
        return auth_admin_request(conn, acp);
      }
      if (memcmp(conn->uri, "/admin/reboot", 13) == 0)
      {
        return auth_admin_reboot(conn, acp);
      }
      if (memcmp(conn->uri, "/admin/reset", 12) == 0)
      {
        return auth_admin_reset(conn, acp);
      }
      if (memcmp(conn->uri, "/admin/setConfig", 16) == 0)
      {
        return auth_admin_set_config(conn, acp);
      }
      if (memcmp(conn->uri, "/admin/getConfig", 16) == 0)
      {
        return auth_admin_get_config(conn, acp);
      }

      return MG_FALSE;

    case MG_POLL:
      printf("MG_POLL conn=%p\n", conn);
      if (conn->connection_param == NULL)
        return MG_FALSE;
      acp = (struct auth_conn_param *)conn->connection_param;
      if (acp->status == WEB_AUTH_INDEX)
      {
        return MG_FALSE;
      }
      else if (acp->status == WEB_AUTH_LOGIN_REQUEST)
      {
        if (acp->time + AUTH_REQUEST_TIMEOUT_SECONDS < current_time)
        {
          mg_printf_data(conn, "%s", "202 reply timeout!\n");
          return MG_TRUE;
        }
        return MG_MORE;
      }
      else if (acp->status == AUTH_USER_ONLINE_REQUEST)
      {
        if (acp->time + AUTH_REQUEST_TIMEOUT_SECONDS < current_time)
        {
          mg_printf_data(conn, "%s", "202 reply timeout!\n");
          return MG_TRUE;
        }
        return MG_MORE;
      }
      else if (acp->status == WEB_AUTH_DONE)
      {
        return MG_TRUE;
      }
      return MG_TRUE;

    case MG_CLOSE:
      printf("MG_CLOSE conn=%p\n", conn);
      acp = (struct auth_conn_param *)conn->connection_param;
      if (acp)
      {
        conn->connection_param = NULL;
        free(acp);
      }
      return MG_TRUE;

    default:
      return MG_FALSE;
  }
}

static inline int auth_login_error(struct mg_connection *conn, const char *msg)
{
  mg_printf_data(conn, "login error: %s\n", msg);
  return MG_TRUE;
}

static int auth_login_request(struct mg_connection *conn, struct auth_conn_param *acp)
{
  unsigned int n;
  unsigned int a, b, c, d, e, f;
  char buf[1024];

  n = mg_get_var(conn, "ip", buf, sizeof(buf));
  if (sscanf(buf, "%u.%u.%u.%u", &a, &b, &c, &d) != 4)
  {
    printf("ip invalid");
    return auth_login_error(conn, "ip invalid");
  }
  acp->req.ip = htonl((a << 24) | (b << 16) | (c << 8) | d);

  n = mg_get_var(conn, "mac", buf, sizeof(buf));
  if (sscanf(buf, "%2X-%2X-%2X-%2X-%2X-%2X", &a, &b, &c, &d, &e, &f) != 6)
  {
    printf("mac invalid");
    return auth_login_error(conn, "mac invalid");
  }
  acp->req.mac[0] = a & 0xFF;
  acp->req.mac[1] = b & 0xFF;
  acp->req.mac[2] = c & 0xFF;
  acp->req.mac[3] = d & 0xFF;
  acp->req.mac[4] = e & 0xFF;
  acp->req.mac[5] = f & 0xFF;

  n = mg_get_var(conn, "username", buf, sizeof(buf));
  if (n > sizeof(acp->req.username) - 1)
  {
    printf("username invalid");
    return auth_login_error(conn, "username invalid");
  }
  strcpy(acp->req.username, buf);

  n = mg_get_var(conn, "password", buf, sizeof(buf));
  if (n > sizeof(acp->req.password) - 1)
  {
    printf("password invalid");
    return auth_login_error(conn, "passowrd invalid");
  }
  strcpy(acp->req.password, buf);

  n = mg_get_var(conn, "ssid", buf, sizeof(buf));
  if (n > sizeof(acp->req.ssid) - 1)
  {
    printf("ssid invalid");
    return auth_login_error(conn, "ssid invalid");
  }
  strcpy(acp->req.ssid, buf);

  acp->status = WEB_AUTH_LOGIN_REQUEST;
  acp->req.id = ++auth_req_id;
  acp->req.flags = AUTH_USER_LOGIN_REQUEST;
  mg_client_send(s_auth_server_conn, &acp->req, sizeof(acp->req));
  return MG_MORE;
}

static int auth_online_request(struct mg_connection *conn, struct auth_conn_param *acp)
{
  unsigned int n;
  unsigned int a, b, c, d, e, f;
  char buf[1024];

  n = mg_get_var(conn, "ip", buf, sizeof(buf));
  if (sscanf(buf, "%u.%u.%u.%u", &a, &b, &c, &d) != 4)
  {
    printf("ip invalid");
    return auth_login_error(conn, "ip invalid");
  }
  acp->req.ip = htonl((a << 24) | (b << 16) | (c << 8) | d);

  n = mg_get_var(conn, "mac", buf, sizeof(buf));
  if (sscanf(buf, "%2X-%2X-%2X-%2X-%2X-%2X", &a, &b, &c, &d, &e, &f) != 6)
  {
    printf("mac invalid");
    return auth_login_error(conn, "mac invalid");
  }
  acp->req.mac[0] = a & 0xFF;
  acp->req.mac[1] = b & 0xFF;
  acp->req.mac[2] = c & 0xFF;
  acp->req.mac[3] = d & 0xFF;
  acp->req.mac[4] = e & 0xFF;
  acp->req.mac[5] = f & 0xFF;

  n = mg_get_var(conn, "ssid", buf, sizeof(buf));
  if (n > sizeof(acp->req.ssid) - 1)
  {
    printf("ssid invalid");
    return auth_login_error(conn, "ssid invalid");
  }
  strcpy(acp->req.ssid, buf);

  acp->status = WEB_AUTH_ONLINE_REQUEST;
  acp->req.id = ++auth_req_id;
  acp->req.flags = AUTH_USER_ONLINE_REQUEST;
  mg_client_send(s_auth_server_conn, &acp->req, sizeof(acp->req));
  return MG_MORE;
}

static int auth_admin_request(struct mg_connection *conn, struct auth_conn_param *acp)
{
  unsigned int n;
  char cmd[128];
  char password[128];

  n = mg_get_var(conn, "cmd", cmd, sizeof(cmd));
  n = mg_get_var(conn, "password", password, sizeof(password));

  acp->status = WEB_AUTH_ADMIN_REQUEST;

  if (strcmp(cmd, "change-password") == 0 && strlen(password) > 0)
  {
    if (modify_passwords_file(HTTPD_PASSWORDS_PATH, AUTH_DOMAIN, "admin", password) == 0)
    {
      mg_printf_data(conn, "200 change password ok");
    }
    else
    {
      mg_printf_data(conn, "202 change password failed");
    }
    return MG_TRUE;
  }

  mg_printf_data(conn, "404 nothing to do");
  return MG_TRUE;
}

static int auth_admin_reboot(struct mg_connection *conn, struct auth_conn_param *acp)
{
  char buf[512];

  acp->status = WEB_AUTH_ADMIN_REQUEST;

  cJSON *msgJS = cJSON_CreateObject();
  cJSON *pldJS = cJSON_CreateObject();
  cJSON *dataJS = cJSON_CreateObject();

  cJSON_AddItemToObject(msgJS, "pld", pldJS);
  cJSON_AddStringToObject(pldJS, "cmd", "ap_reboot");
  cJSON_AddItemToObject(pldJS, "data", dataJS);

  char *msgStr = cJSON_PrintUnformatted(msgJS);
  printf("%s\n", msgStr);
  //mqttpub 'a/local/cfgmgr' 'msgStr'
  sprintf(buf, "/ugw/scripts/mqttpub 'a/local/cfgmgr' '%s'", msgStr);
  system(buf);

  free(msgStr);
  cJSON_Delete(msgJS);

  mg_printf_data(conn, "{\"state\": true,\"message\": \"reboot ok\"}");

  return MG_TRUE;
}

static int auth_admin_reset(struct mg_connection *conn, struct auth_conn_param *acp)
{
  char buf[512];

  acp->status = WEB_AUTH_ADMIN_REQUEST;

  cJSON *msgJS = cJSON_CreateObject();
  cJSON *pldJS = cJSON_CreateObject();
  cJSON *dataJS = cJSON_CreateObject();

  cJSON_AddItemToObject(msgJS, "pld", pldJS);
  cJSON_AddStringToObject(pldJS, "cmd", "ap_reset");
  cJSON_AddItemToObject(pldJS, "data", dataJS);

  char *msgStr = cJSON_PrintUnformatted(msgJS);
  printf("%s\n", msgStr);
  //mqttpub 'a/local/cfgmgr' 'msgStr'
  sprintf(buf, "/ugw/scripts/mqttpub 'a/local/cfgmgr' '%s'", msgStr);
  system(buf);

  free(msgStr);
  cJSON_Delete(msgJS);

  mg_printf_data(conn, "{\"state\": true,\"message\": \"reset ok\"}");

  return MG_TRUE;
}

static int auth_admin_set_config(struct mg_connection *conn, struct auth_conn_param *acp)
{
  int n;
  char et0macaddr[24];
  char cloud_account[128];
  int lan_dhcp;
  char lan_ipaddr[24];
  char lan_netmask[24];
  char lan_gateway[24];
  char achost_ip[64];
  char buf[1024];
  conn = conn;
  acp->status = WEB_AUTH_ADMIN_REQUEST;

//FIXME
  n = mg_get_var(conn, "et0macaddr", et0macaddr, sizeof(et0macaddr));
  n = mg_get_var(conn, "cloud_account", cloud_account, sizeof(cloud_account));
  n = mg_get_var(conn, "lan_dhcp", buf, sizeof(buf));
  lan_dhcp = atol(buf);
  n = mg_get_var(conn, "lan_ipaddr", lan_ipaddr, sizeof(lan_ipaddr));
  n = mg_get_var(conn, "lan_netmask", lan_netmask, sizeof(lan_netmask));
  n = mg_get_var(conn, "lan_gateway", lan_gateway, sizeof(lan_gateway));
  n = mg_get_var(conn, "achost_ip", achost_ip, sizeof(achost_ip));
  n = mg_get_var(conn, "dns", buf, sizeof(buf));

  cJSON *msgJS = cJSON_CreateObject();
  cJSON *pldJS = cJSON_CreateObject();
  cJSON *dataJS = cJSON_CreateObject();

  cJSON_AddItemToObject(msgJS, "pld", pldJS);
  cJSON_AddStringToObject(pldJS, "cmd", "ap_set_config");
  cJSON_AddItemToObject(pldJS, "data", dataJS);
  cJSON_AddStringToObject(dataJS, "et0macaddr", et0macaddr);
  cJSON_AddStringToObject(dataJS, "cloud_account", cloud_account);
  cJSON_AddStringToObject(dataJS, "lan_ipaddr", lan_ipaddr);
  cJSON_AddStringToObject(dataJS, "lan_netmask", lan_netmask);
  cJSON_AddStringToObject(dataJS, "lan_gateway", lan_gateway);
  cJSON_AddStringToObject(dataJS, "achost_ip", achost_ip);
  cJSON_AddStringToObject(dataJS, "dns", buf);
  cJSON_AddNumberToObject(dataJS, "lan_dhcp", lan_dhcp);

  char *msgStr = cJSON_PrintUnformatted(msgJS);
  printf("%s\n", msgStr);
  //mqttpub 'a/local/cfgmgr' 'msgStr'
  sprintf(buf, "/ugw/scripts/mqttpub 'a/local/cfgmgr' '%s'", msgStr);
  system(buf);

  free(msgStr);
  cJSON_Delete(msgJS);

  mg_printf_data(conn, "{\"state\": true,\"message\": \"config saved\"}");

  return MG_TRUE;
}

static const char *__local_hwaddr(char *eth)
{
	int fd; 
	struct ifreq ifr;

  static char macaddr[32];

  macaddr[0] = 0;

	if ((fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) 
	{   
    return macaddr;
	}   

	strncpy(ifr.ifr_name, eth, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';

	if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0)
	{   
		close(fd);
    return macaddr;
	}

	close(fd);

  sprintf(macaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
      ifr.ifr_hwaddr.sa_data[0] & 0xFF,
      ifr.ifr_hwaddr.sa_data[1] & 0xFF,
      ifr.ifr_hwaddr.sa_data[2] & 0xFF,
      ifr.ifr_hwaddr.sa_data[3] & 0xFF,
      ifr.ifr_hwaddr.sa_data[4] & 0xFF,
      ifr.ifr_hwaddr.sa_data[5] & 0xFF);

  return macaddr;
}

static int auth_admin_get_config(struct mg_connection *conn, struct auth_conn_param *acp)
{
  int cloud_connected = cJSON_False;
  cJSON *ac_host1JS = NULL;

  cJSON *remote_stateJS = auth_load_cJSON("/tmp/memfile/remote_state.json");
  cJSON *ap_configJS = auth_load_cJSON("/ugwconfig/etc/ap/ap_config.json");

  cJSON *cfgJS = cJSON_CreateObject();
  cJSON *sysconfigJS = cJSON_CreateObject();

  acp->status = WEB_AUTH_ADMIN_REQUEST;

  if (remote_stateJS)
  {
    cJSON *cloud_connectedJS = cJSON_GetObjectItem(remote_stateJS, "connecting");
    if (cloud_connectedJS)
    {
      if (cloud_connectedJS->type == cJSON_True)
      {
        cloud_connected = cJSON_True;
      }
    }

    ac_host1JS = cJSON_GetObjectItem(remote_stateJS, "achost");
  }

  if (ap_configJS)
  {
    cJSON *js;
    char *cfgStr;

    js = cJSON_GetObjectItem(ap_configJS, "a#account");
    if (js) cJSON_AddStringToObject(sysconfigJS, "cloud_account", js->valuestring);
    else cJSON_AddStringToObject(sysconfigJS, "cloud_account", "default");

    js = cJSON_GetObjectItem(ap_configJS, "a#distr");
    if (js && strcmp(js->valuestring, "dhcp") == 0) cJSON_AddNumberToObject(sysconfigJS, "lan_dhcp", 1);
    else cJSON_AddNumberToObject(sysconfigJS, "lan_dhcp", 0);

    js = cJSON_GetObjectItem(ap_configJS, "a#ip");
    if (js) cJSON_AddStringToObject(sysconfigJS, "lan_ipaddr", js->valuestring);
    else cJSON_AddStringToObject(sysconfigJS, "lan_ipaddr", "");

    js = cJSON_GetObjectItem(ap_configJS, "a#mask");
    if (js) cJSON_AddStringToObject(sysconfigJS, "lan_netmask", js->valuestring);
    else cJSON_AddStringToObject(sysconfigJS, "lan_netmask", "");

    js = cJSON_GetObjectItem(ap_configJS, "a#gw");
    if (js) cJSON_AddStringToObject(sysconfigJS, "lan_gateway", js->valuestring);
    else cJSON_AddStringToObject(sysconfigJS, "lan_gateway", "");

    js = cJSON_GetObjectItem(ap_configJS, "a#dns");
    if (js) cJSON_AddStringToObject(sysconfigJS, "dns", js->valuestring);
    else cJSON_AddStringToObject(sysconfigJS, "dns", "");

    js = cJSON_GetObjectItem(ap_configJS, "a#ac_host");
    if (js) cJSON_AddStringToObject(sysconfigJS, "achost_ip", js->valuestring);
    else cJSON_AddStringToObject(sysconfigJS, "achost_ip", "");

    js = ac_host1JS;
    if (js) cJSON_AddStringToObject(sysconfigJS, "current_mqttip", js->valuestring);
    else cJSON_AddStringToObject(sysconfigJS, "current_mqttip", "");

    cJSON_AddStringToObject(sysconfigJS, "et0macaddr", __local_hwaddr("eth0"));

    cJSON_AddBoolToObject(sysconfigJS, "cloud_connected", cloud_connected);

    cJSON_AddItemToObject(cfgJS, "sysconfig", sysconfigJS);

    cfgStr = cJSON_PrintUnformatted(cfgJS);

    printf("\n%s\n", cfgStr);
    mg_printf_data(conn, cfgStr);

    free(cfgStr);
    cJSON_Delete(cfgJS);
    cJSON_Delete(ap_configJS);
  }
  else
  {
    mg_printf_data(conn, "{\"state\": false,\"message\": \"no config data\"}");
  }

  if (remote_stateJS)
    cJSON_Delete(remote_stateJS);

  return MG_TRUE;
}

static void show_usage_and_exit(void)
{
  fprintf(stderr, "Mongoose version %s (c) Sergey Lyubka, built on %s\n",
      MONGOOSE_VERSION, __DATE__);
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  httpd -A <htpasswd_file> <realm> <user> <passwd>\n");

  exit(EXIT_FAILURE);
}

static int modify_passwords_file(const char *fname, const char *domain, const char *user, const char *pass)
{
  int found;
  char line[512], u[512], d[512], ha1[33], tmp[PATH_MAX];
  FILE *fp, *fp2;

  found = 0;
  fp = fp2 = NULL;

  // Regard empty password as no password - remove user record.
  if (pass != NULL && pass[0] == '\0') {
    pass = NULL;
  }

  (void) snprintf(tmp, sizeof(tmp), "%s.tmp", fname);

  // Create the file if does not exist
  if ((fp = fopen(fname, "a+")) != NULL) {
    fclose(fp);
  }

  // Open the given file and temporary file
  if ((fp = fopen(fname, "r")) == NULL) {
    return -1;
  } else if ((fp2 = fopen(tmp, "w+")) == NULL) {
    fclose(fp);
    return -1;
  }

  // Copy the stuff to temporary file
  while (fgets(line, sizeof(line), fp) != NULL) {
    if (sscanf(line, "%[^:]:%[^:]:%*s", u, d) != 2) {
      continue;
    }

    if (!strcmp(u, user) && !strcmp(d, domain)) {
      found++;
      if (pass != NULL) {
        mg_md5(ha1, user, ":", domain, ":", pass, NULL);
        fprintf(fp2, "%s:%s:%s\n", user, domain, ha1);
      }
    } else {
      fprintf(fp2, "%s", line);
    }
  }

  // If new user, just add it
  if (!found && pass != NULL) {
    mg_md5(ha1, user, ":", domain, ":", pass, NULL);
    fprintf(fp2, "%s:%s:%s\n", user, domain, ha1);
  }

  // Close files
  fclose(fp);
  fclose(fp2);

  // Put the temp file in place of real file
  remove(fname);
  rename(tmp, fname);

  return 0;
}

int main(int argc, char **argv) {

  // Edit passwords file if -A option is specified
  if (argc > 1 && !strcmp(argv[1], "-A"))
  {
    if (argc != 6)
    {
      show_usage_and_exit();
    }
    exit(modify_passwords_file(argv[2], argv[3], argv[4], argv[5]) ? EXIT_SUCCESS : EXIT_FAILURE);
  }

	if (access(HTTPD_PASSWORDS_PATH, R_OK|F_OK))
	{
    modify_passwords_file(HTTPD_PASSWORDS_PATH, AUTH_DOMAIN, "admin", "admin");
	}
  
  if (access("/tmp/webui/admin/index.html", R_OK|F_OK))
  {
    system("mkdir -p /tmp/webui; /bin/tar -C /tmp/webui -xf /ugw/webui/admin.tar");
  }
  if (access("/tmp/webui/index.html", R_OK|F_OK))
  {
    system("mkdir -p /tmp/webui; /bin/tar -C /tmp/webui -xf /ugw/webui/webui.tar");
  }

  s_server = mg_create_server(NULL, ev_handler);

  mg_set_option(s_server, "listening_port", "80");
  mg_set_option(s_server, "document_root", "/tmp/webui");
  mg_set_option(s_server, "auth_domain", AUTH_DOMAIN);

  // Setup signal handlers
  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);

  s_auth_server_conn = mg_client_udp_connect(s_server, s_auth_server_addr);
  printf("s_auth_server_conn=%p\n", s_auth_server_conn);

  printf("Listening on port %s\n", mg_get_option(s_server, "listening_port"));
  while (s_received_signal == 0) {
    mg_poll_server(s_server, 1000);
  }
  mg_destroy_server(&s_server);
  printf("Existing on signal %d\n", s_received_signal);

  return EXIT_SUCCESS;
}
