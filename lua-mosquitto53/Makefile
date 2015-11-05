include $(TOPDIR)/rules.mk

PKG_NAME:=lua-mosquitto53
PKG_VERSION:=0.2
PKG_RELEASE:=1

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=
PKG_MAINTAINER:=roberto

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=Applications
	CATEGORY:=apps
	TITLE:=lua-mosquitto14
	DEPENDS:=+lua53 mosquitto14
endef

define Package/$(PKG_NAME)/description
	lua binding for mosquitto14 
endef

define Build/Compile  
	$(MAKE) -C lua-mosquitto-master $(TARGET_CONFIGURE_OPTS) LUA_SO_DIR=$(BUILD_DIR)/lua53/src MQTT_SO_DIR=$(BUILD_DIR)/mosquitto14/lib/
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/lua/
	$(INSTALL_BIN) lua-mosquitto-master/mosquitto.so $(1)/usr/lib/lua/ 
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
