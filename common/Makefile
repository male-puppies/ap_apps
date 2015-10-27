#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=common
PKG_VERSION:=2015-10-27
PKG_RELEASE:=1

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=
PKG_MAINTAINER:=Ivan <itgb1989@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/common
	SECTION:=Applications
	CATEGORY:=Puppies
	TITLE:=self init script
endef

define Package/common/description
	common system 
endef


define Build/Prepare
	true
endef

define Build/Compile
	true
endef

define Package/common/install 
	$(CP) -a ./root/* $(1)/
endef

define Package/common/postinst
	true
	exit 0
endef

define Package/common/prerm
	true
	exit 0
endef

$(eval $(call BuildPackage,common))
