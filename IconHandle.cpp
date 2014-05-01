#include "IconHandle.h"

namespace JsNotifyIcon {

	IconHandle::IconHandle(HICON handle) {
		this->_handle = handle;
	}

	IconHandle::~IconHandle() {
		DestroyIcon(this->_handle);
	}

	HICON IconHandle::Handle() {
		return this->_handle;
	}

}