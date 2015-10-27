include $(TOPDIR)/rules.mk

PKG_NAME:=see
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
	TITLE:=see
	#DEPENDS:=+liblua
endef

define Package/$(PKG_NAME)/description
	se library
endef

define Build/Compile
	$(MAKE) -C se  $(TARGET_CONFIGURE_OPTS)
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/ugw/script/
	$(INSTALL_BIN) se/bin/*so $(1)/usr/lib/
	$(INSTALL_BIN) se/bin/*.lua $(1)/ugw/script/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
