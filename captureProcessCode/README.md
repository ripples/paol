Capture from all whiteboard cameras and vga2usb:
	cameraCapture.cpp
	-compiles to: camCap
	KNOWN ISSUES:
	      this will sometimes fail to capture some devices when
	      started multiple times by script

Capture from all whiteboard cameras and vga2usb devices one program
	multiDeviceCapture.cpp
	-compiles to:multiCap
	KNOWN ISSUE:
	      since everything is captured with a single thread
	      it can be slow and doesn't lend itself to realtime processing
	      (it's ugly but works consistently)

Process an individual whiteboard camera
	wbMain.cpp
	-compiles to: processWB

Process an individual vga2usb stream
	compMain.cpp
	-compiles to: processCOMP
