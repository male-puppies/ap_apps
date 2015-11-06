include $(TOPDIR)/rules.mk

PKG_NAME:=luasocket53
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
	TITLE:=luasocket53
	DEPENDS:=+lua53
endef

define Package/$(PKG_NAME)/description
	lua binding for socket 
endef

define Build/Compile 
	$(MAKE) -C luasocket-master LUA_SO_DIR=$(BUILD_DIR)/lua53/src $(TARGET_CONFIGURE_OPTS) 
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/lua/socket
	$(INSTALL_DIR) $(1)/usr/lib/lua/mime
	$(INSTALL_DATA) luasocket-master/src/socket-3.0-rc1.so $(1)/usr/lib/lua/socket/core.so
	$(INSTALL_DATA) luasocket-master/src/mime-1.0.3.so $(1)/usr/lib/lua/mime/core.so
	cd luasocket-master/src/ && $(INSTALL_DATA) *.lua $(1)/usr/lib/lua 
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
