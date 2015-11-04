include $(TOPDIR)/rules.mk

PKG_NAME:=lfs
PKG_VERSION:=0.1
PKG_RELEASE:=0

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=
PKG_MAINTAINER:=Ivan <itgb1989@gmail.com>

SRC_DIR:=luafilesystem-master 
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=Applications
	CATEGORY:=apps
	TITLE:=lua filesystem library
	DEPENDS:=+lua53
endef

define Package/$(PKG_NAME)/description
	lua filesystem library
endef

define Build/Compile 
	$(MAKE) -C luafilesystem-master/src LUA_SO_DIR=$(BUILD_DIR)/lua53/src $(TARGET_CONFIGURE_OPTS)   
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/lua
	#$(CP) $(PKG_BUILD_DIR)/src/lfs.so  $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
