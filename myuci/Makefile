include $(TOPDIR)/rules.mk

PKG_NAME:=myuci
PKG_VERSION:=0.1
PKG_RELEASE:=0

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=
PKG_MAINTAINER:=Ivan <itgb1989@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
SRC_DIR:=src

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=Applications
	CATEGORY:=apps
	TITLE:=myuci
	DEPENDS:=+libuci
endef

define Package/$(PKG_NAME)/description
	myuci
endef

define Build/Compile
	$(MAKE) -C $(SRC_DIR) LUA_SO_DIR=$(BUILD_DIR)/lua53/src $(TARGET_CONFIGURE_OPTS)  
endef

define Package/$(PKG_NAME)/install   
	$(INSTALL_DIR) $(1)/usr/lib/lua/
	$(INSTALL_DATA) src/uci53.so $(1)/usr/lib/lua/ 
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
