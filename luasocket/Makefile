include $(TOPDIR)/rules.mk

PKG_NAME:=luasocket
PKG_VERSION:=0.1
PKG_RELEASE:=0

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=
PKG_MAINTAINER:=Ivan <itgb1989@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

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
	$(MAKE) -C luasocket-2.0.2  $(TARGET_CONFIGURE_OPTS)
endef

define Package/$(PKG_NAME)/install  
	$(INSTALL_DIR) $(1)/usr/lib/socket
	$(INSTALL_BIN) luasocket-2.0.2/src/socket.so.2.0.2  $(1)/usr/lib/socket/core.so
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
