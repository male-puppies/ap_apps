include $(TOPDIR)/rules.mk

PKG_NAME:=mqtt
PKG_VERSION:=0.1
PKG_RELEASE:=0

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=
PKG_MAINTAINER:=Ivan <itgb1989@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
SRC_DIR:= mosquitto-1.4.2

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=Applications
	CATEGORY:=apps
	TITLE:=mqtt library
	#DEPENDS:=+liblua
endef

define Package/$(PKG_NAME)/description
	mqtt library
endef

define Build/Compile
	$(MAKE) -C mosquitto-1.4.2  $(TARGET_CONFIGURE_OPTS)
	
	mkdir -p $(PKG_BUILD_DIR)/lib
	mkdir -p $(PKG_BUILD_DIR)/src
	$(CP) $(SRC_DIR)/lib/libmosquitto.so.1 $(PKG_BUILD_DIR)/lib
	ln -sf $(PKG_BUILD_DIR)/lib/libmosquitto.so.1 $(PKG_BUILD_DIR)/lib/libmosquitto.so
	$(CP) $(SRC_DIR)/src/mosquitto $(PKG_BUILD_DIR)/src
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/lib/libmosquitto.so.1 $(1)/usr/lib/
	ln -sf $(1)/usr/lib/libmosquitto.so.1 $(1)/usr/lib/libmosquitto.so 
	$(CP) $(PKG_BUILD_DIR)/src/mosquitto $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
