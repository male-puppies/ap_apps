include $(TOPDIR)/rules.mk

PKG_NAME:=luv53
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
	TITLE:=luv53
	DEPENDS:=+lua53 libuv
endef

define Package/$(PKG_NAME)/description
	lua binding for libuv 
endef

define Build/Compile  
	$(MAKE) -C luv/src $(TARGET_CONFIGURE_OPTS) LUA_SO_DIR=$(BUILD_DIR)/lua53/src  LIBUV_INC=$(BUILD_DIR)/libuv-v1.6.1/include/ LIBUV_LIB=$(BUILD_DIR)/libuv-v1.6.1/.libs/ 
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/lua/
	$(INSTALL_BIN) luv/src/luv53.so $(1)/usr/lib/lua/ 
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
