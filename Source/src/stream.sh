gst-launch -v v4l2src always-copy=FALSE ! video/x-raw-yuv,format=\(fourcc\)NV12, width=640, height=480 ! dmaiaccel ! queue ! dmaienc_mpeg4 ! udpsink host=$1 port=$2
