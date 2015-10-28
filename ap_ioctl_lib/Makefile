include $(TOPDIR)/rules.mk

PKG_NAME:=apioctl
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
	TITLE:=ap ioctl lib
	DEPENDS:=+liblua
endef

define Package/$(PKG_NAME)/description
	ap ioctl library
endef

define Build/Compile
	$(MAKE) -C ap_ioctl_lib  $(TARGET_CONFIGURE_OPTS)
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_BIN) ap_ioctl_lib/*.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
