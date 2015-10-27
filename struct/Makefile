include $(TOPDIR)/rules.mk

PKG_NAME:=struct
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
	TITLE:=struct library
	DEPENDS:=+liblua
endef

define Package/$(PKG_NAME)/description
	struct library
endef

define Build/Compile
	$(MAKE) -C struct-0.2  $(TARGET_CONFIGURE_OPTS)
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_BIN) struct-0.2/struct.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
