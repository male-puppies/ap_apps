include $(TOPDIR)/rules.mk

PKG_NAME:=apserver
PKG_VERSION:=0.1
PKG_RELEASE:=0

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=
PKG_MAINTAINER:=Ivan <itgb1989@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
SRC_DIR:=ap_server
include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=Applications
	CATEGORY:=apps
	TITLE:=ap server
	#DEPENDS:=+liblua
endef

define Package/$(PKG_NAME)/description
	ap server
endef

define Build/Compile
	$(MAKE) -C $(SRC_DIR)  $(TARGET_CONFIGURE_OPTS)

	mkdir -p $(PKG_BUILD_DIR)/ugw/bin/	
	$(CP) $(SRC_DIR)/httpd $(PKG_BUILD_DIR)/ugw/bin/
	$(CP) $(SRC_DIR)/authd $(PKG_BUILD_DIR)/ugw/bin/
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/ugw/bin/httpd $(1)/ugw/bin/
	chmod 0755 $(1)/ugw/bin/httpd
	$(CP) $(PKG_BUILD_DIR)/ugw/bin/authd $(1)/ugw/bin/
	chmod 0755 $(1)/ugw/bin/authd
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
