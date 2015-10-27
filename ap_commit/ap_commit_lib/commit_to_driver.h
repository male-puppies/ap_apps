#ifndef _COMMIT_TO_DRIVER_H_
#define _COMMIT_TO_DRIVER_H_

int32_t full_commit_per_radio(struct wl_radio_st *radio);

int32_t radio_up(struct wl_radio_st *radio);
int32_t radio_down(struct wl_radio_st *radio);
int32_t radio_cfg_commit(struct wl_radio_st *radio);

int32_t destroy_all_vap(struct wl_radio_st *radio);
int32_t create_all_vap(struct wl_radio_st *radio);
int32_t set_all_vap(struct wl_radio_st *radio);
int32_t set_all_vap_enable(struct wl_radio_st *radio);

int32_t custom_popen(const char *cmd_str, const uint16_t cmd_len, 
							char *result, const uint16_t len);

#endif
