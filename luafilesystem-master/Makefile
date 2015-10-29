include $(TOPDIR)/rules.mk

PKG_NAME:=lfs
PKG_VERSION:=0.1
PKG_RELEASE:=0

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=
PKG_MAINTAINER:=Ivan <itgb1989@gmail.com>

SRC_DIR:=luafilesystem-master 
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=Applications
	CATEGORY:=apps
	TITLE:=lua filesystem library
	DEPENDS:=+liblua51
endef

define Package/$(PKG_NAME)/description
	lua filesystem library
endef

define Build/Compile
	#$(MAKE) -C luafilesystem-master  $(TARGET_CONFIGURE_OPTS)
	$(MAKE)  -C $(SRC_DIR) \
		CC="$(TARGET_CROSS)gcc" \
		AR="$(TARGET_CROSS)ar rcu" \
		RANLIB="$(TARGET_CROSS)ranlib" \
		INSTALL_ROOT=/usr \
		CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
		MYLDFLAGS="$(TARGET_LDFLAGS)" \
		PKG_VERSION=$(PKG_VERSION) \
		LUA_LIBDIR=$(BUILD_DIR)/lua51/src
	#mkdir -p $(PKG_BUILD_DIR)/src
	#$(CP) $(SRC_DIR)/src/lfs.so $(PKG_BUILD_DIR)/src/
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/
	#$(CP) $(PKG_BUILD_DIR)/src/lfs.so  $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
