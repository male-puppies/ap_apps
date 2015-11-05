include $(TOPDIR)/rules.mk

PKG_NAME:=lua-cjson53
PKG_VERSION:=0.1
PKG_RELEASE:=0

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=
PKG_MAINTAINER:=Ivan <itgb1989@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
SRC_DIR:=lua-cjson-master

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=Applications
	CATEGORY:=apps
	TITLE:=lua cjson
	DEPENDS:=+lua53
endef

define Package/$(PKG_NAME)/description
	lua cjson master
endef

define Build/Compile
	$(MAKE) -C lua-cjson-master  $(TARGET_CONFIGURE_OPTS) 
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DATA) lua-cjson-master/cjson53.so $(1)/usr/lib/lua
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
