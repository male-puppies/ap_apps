include $(TOPDIR)/rules.mk

PKG_NAME:=lua-cjson-master
PKG_VERSION:=0.1
PKG_RELEASE:=0

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=
PKG_MAINTAINER:=Ivan <itgb1989@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
SRC_DIR:=lua-cjson-master

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=Applications
	CATEGORY:=apps
	TITLE:=lua cjson master
	DEPENDS:=+lua51 +liblua
endef

define Package/$(PKG_NAME)/description
	lua cjson master
endef

define Build/Compile
	$(MAKE) -C lua-cjson-master  $(TARGET_CONFIGURE_OPTS)

	#$(MAKE) -C $(SRC_DIR) $(TARGET_CONFIGURE_OPTS) \
	#	LUA51_SO_DIR=$(BUILD_DIR)/lua51/src  LUA_INC=$(BUILD_DIR)/lua51/src
	#$(CP) $(SRC_DIR)/cjson.so $(PKG_BUILD_DIR)
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(CP) $(PKG_BUILD_DIR)/cjson.so $(1)/usr/lib/lua
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
