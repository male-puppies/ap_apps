include $(TOPDIR)/rules.mk

PKG_NAME:=lfs
PKG_VERSION:=0.1
PKG_RELEASE:=0

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=
PKG_MAINTAINER:=Ivan <itgb1989@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=Applications
	CATEGORY:=apps
	TITLE:=lua filesystem library
	DEPENDS:=+liblua
endef

define Package/$(PKG_NAME)/description
	lua filesystem library
endef

define Build/Compile
	$(MAKE) -C luafilesystem-master  $(TARGET_CONFIGURE_OPTS)
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_BIN) luafilesystem-master/src/lfs.so  $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
