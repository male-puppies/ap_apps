include $(TOPDIR)/rules.mk

PKG_NAME:=ap_netlink
PKG_VERSION:=0.1
PKG_RELEASE:=0

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=
PKG_MAINTAINER:=Ivan <itgb1989@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
SRC_DIR:=ap_netlink_lib
include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=Applications
	CATEGORY:=apps
	TITLE:=ap netlink library
	#DEPENDS:=+liblua
endef

define Package/$(PKG_NAME)/description
	ap netlink library
endef

define Build/Compile
	$(MAKE) -C $(SRC_DIR) $(TARGET_CONFIGURE_OPTS)
		
	$(CP) $(SRC_DIR)/*.so $(PKG_BUILD_DIR)
	$(CP) $(SRC_DIR)/test_netlink $(PKG_BUILD_DIR)
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/*.so $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/test_netlink $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
