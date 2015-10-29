include $(TOPDIR)/rules.mk

PKG_NAME:=see
PKG_VERSION:=0.1
PKG_RELEASE:=0

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=
PKG_MAINTAINER:=Ivan <itgb1989@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
SRC_DIR:= se

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=Applications
	CATEGORY:=apps
	TITLE:=see
	DEPENDS:=+liblua
endef

define Package/$(PKG_NAME)/description
	se library
endef

define Build/Compile
	$(MAKE) -C $(SRC_DIR)  $(TARGET_CONFIGURE_OPTS)
	#$(MAKE)  -C $(SRC_DIR) 
		
	mkdir -p $(PKG_BUILD_DIR)/bin
	$(CP) $(SRC_DIR)/bin/*.so $(PKG_BUILD_DIR)/bin
	$(CP) $(SRC_DIR)/bin/*.lua $(PKG_BUILD_DIR)/bin
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/ugw/script/
	$(CP) $(PKG_BUILD_DIR)/bin/*so $(1)/usr/lib/
	$(CP)  $(PKG_BUILD_DIR)/bin/*.lua $(1)/ugw/script/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
