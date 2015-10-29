include $(TOPDIR)/rules.mk

PKG_NAME:=luasocket
PKG_VERSION:=0.1
PKG_RELEASE:=0

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=
PKG_MAINTAINER:=Ivan <itgb1989@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
SRC_DIR:=luasocket-2.0.2

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=Applications
	CATEGORY:=apps
	TITLE:=lua socket library
	DEPENDS:=+liblua
endef

define Package/$(PKG_NAME)/description
	lua socket library
endef

define Build/Compile
	#$(MAKE) -C luasocket-2.0.2  $(TARGET_CONFIGURE_OPTS)
	$(MAKE)  -C $(SRC_DIR) \
		CC="$(TARGET_CROSS)gcc" \
		AR="$(TARGET_CROSS)ar rcu" \
		RANLIB="$(TARGET_CROSS)ranlib" \
		INSTALL_ROOT=/usr \
		CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
		MYLDFLAGS="$(TARGET_LDFLAGS)" \
		PKG_VERSION=$(PKG_VERSION)
	mkdir -p $(PKG_BUILD_DIR)/src
	$(CP) $(SRC_DIR)/src/socket.so.2.0.2 $(PKG_BUILD_DIR)
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/socket
	$(CP) $(PKG_BUILD_DIR)/src/socket.so.2.0.2  $(1)/usr/lib/socket/core.so
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
