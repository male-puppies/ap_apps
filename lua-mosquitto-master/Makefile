include $(TOPDIR)/rules.mk

PKG_NAME:=luamqtt
PKG_VERSION:=0.1
PKG_RELEASE:=0

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=
PKG_MAINTAINER:=Ivan <itgb1989@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
SRC_DIR:=lua-mosquitto-master

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=Applications
	CATEGORY:=apps
	TITLE:=luamqtt
	DEPENDS:=+mqtt + liblua
endef

define Package/$(PKG_NAME)/description
	lua-mosquitto-master library
endef

define Build/Compile
	$(MAKE)  -C $(SRC_DIR) \
		CC="$(TARGET_CROSS)gcc" \
		AR="$(TARGET_CROSS)ar rcu" \
		RANLIB="$(TARGET_CROSS)ranlib" \
		INSTALL_ROOT=/usr \
		CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
		MYLDFLAGS="$(TARGET_LDFLAGS)" \
		PKG_VERSION=$(PKG_VERSION) \
		LUA_INC=$(BUILD_DIR)/lua51/src \
		MQTT_INC=$(BUILD_DIR)/mqtt/lib/
	$(CP) $(SRC_DIR)/mosquitto.so $(PKG_BUILD_DIR)
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/mosquitto.so  $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
