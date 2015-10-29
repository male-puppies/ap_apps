include $(TOPDIR)/rules.mk

PKG_NAME:=redis_parser
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
	TITLE:=redis_parser
	DEPENDS:=+liblua
endef

define Package/$(PKG_NAME)/description
	redis_parser library
endef

define Build/Compile
	#$(MAKE) -C lua-redis-parser  $(TARGET_CONFIGURE_OPTS)
	$(MAKE)  -C $(SRC_DIR) \
		CC="$(TARGET_CROSS)gcc" \
		AR="$(TARGET_CROSS)ar rcu" \
		RANLIB="$(TARGET_CROSS)ranlib" \
		INSTALL_ROOT=/usr \
		CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
		MYLDFLAGS="$(TARGET_LDFLAGS)" \
		PKG_VERSION=$(PKG_VERSION) \
		LUA_INC=$(BUILD_DIR)/lua51/src
	$(INSTALL_DATA) $(SRC_DIR)/parser.so $(PKG_BUILD_DIR)/

endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/redis/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/parser.so $(1)/usr/lib/redis/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
