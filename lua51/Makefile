include $(TOPDIR)/rules.mk

PKG_NAME:=lua51
PKG_VERSION:=0.2
PKG_RELEASE:=1

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=
PKG_MAINTAINER:=

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
SRC_DIR=lua-5.1.5
include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=Applications
	CATEGORY:=apps
	TITLE:=lua 5.1.5
endef

define Package/$(PKG_NAME)/description
	lua 5.1.5
endef

define Build/Compile
	#$(MAKE) -C $(SRC_DIR) linux $(TARGET_CONFIGURE_OPTS)
	$(MAKE)  -C $(SRC_DIR) \
		CC="$(TARGET_CROSS)gcc" \
		AR="$(TARGET_CROSS)ar rcu" \
		RANLIB="$(TARGET_CROSS)ranlib" \
		INSTALL_ROOT=/usr \
		CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
		MYLDFLAGS="$(TARGET_LDFLAGS)" \
		#PKG_VERSION=$(PKG_VERSION) \
		linux
	mkdir -p $(PKG_BUILD_DIR)/src/
	cd $(SRC_DIR)/src/ && cp liblua.so $(PKG_BUILD_DIR)/src/liblua.so
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/lib
	
	cd $(SRC_DIR)/src/ && cp -f lua $(1)/usr/sbin/lua51
	cd $(SRC_DIR)/src/ && cp -f liblua.so $(1)/usr/lib/liblua51.so
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
