include $(TOPDIR)/rules.mk

PKG_NAME:=lua
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
	TITLE:=lua interpreter
	#DEPENDS:=+liblua
endef

define Package/$(PKG_NAME)/description
	lua interpreter
endef

define Build/Compile
	$(MAKE) -C lua-5.1.5  $(TARGET_CONFIGURE_OPTS)
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) lua-5.1.5/src/liblua.so  $(1)/usr/lib/
	$(INSTALL_BIN) lua-5.1.5/src/lua  $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
