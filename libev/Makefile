include $(TOPDIR)/rules.mk

PKG_NAME:=libev
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
	TITLE:=libev
	#DEPENDS:=+liblua
endef

define Package/$(PKG_NAME)/description
	libev event-driven library
endef

define Build/Compile
	$(MAKE) -C libev-4.15  $(TARGET_CONFIGURE_OPTS)
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/lua/
	$(INSTALL_BIN) libev-4.15/.libs/libev.so.4.0.0  $(1)/usr/lib/lua/libev.so.4
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
