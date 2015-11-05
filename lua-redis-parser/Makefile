include $(TOPDIR)/rules.mk

PKG_NAME:=lua_redis_parser
PKG_VERSION:=0.1
PKG_RELEASE:=0

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=
PKG_MAINTAINER:=Ivan <itgb1989@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
SRC_DIR:=lua-redis-parser

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=Applications
	CATEGORY:=apps
	TITLE:=lua_redis_parser
	DEPENDS:=+lua53
endef

define Package/$(PKG_NAME)/description
	redis_parser library
endef

define Build/Compile
	$(MAKE) -C lua-redis-parser LUA_SO_DIR=$(BUILD_DIR)/lua53/src $(TARGET_CONFIGURE_OPTS) 
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/lua/redis/
	$(INSTALL_DATA) lua-redis-parser/parser53.so $(1)/usr/lib/lua/redis/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
