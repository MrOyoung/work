#!/bin/sh
mount -o remount,rw /


if [ -d "./app" ] ;then
	echo "install app..."
	mkdir -p /opt/bin
	cp  app/* /opt/bin
	chmod +x /opt/bin/app
	echo "install app ok..."
fi

if [ -d "./service" ] ;then
	echo "install service..."

	for __file in `ls ./service`
	do
		cp ./service/$(basename ${__file}) /usr/bin
		chmod +x /usr/bin/$(basename ${__file})
		echo "install ${__file} ok"
	done
	
	echo "install service ok..."
fi

if [ -d "./service-script" ] ;then
	echo "install service scripts..."
	cp service-script/*.service /lib/systemd/system/

	#noempty
	cp service-script/noempty /usr/bin/
	chmod +x /usr/bin/noempty

	systemctl enable audiomgr.service
	systemctl enable didata.service
	systemctl enable ipcuart.service
	systemctl enable kanzi.service
	systemctl enable power.service
	systemctl enable sdbus.service
	systemctl enable camera.service
	echo "install service scripts ok..."
fi


echo "shutdown xs-window-switcher.service"
#systemctl disable xs-window-switcher.service //rocky -- 20170613


if [ -d "./xs-conf" ] ;then
	echo "update /etc/wayland/compositor.conf"
	cp -f xs-conf/compositor.conf /etc/wayland
fi


if [ -d "./opencv-wayland-install" ] ;then
	echo "install opencv for wayland..."
	cp -vr ./opencv-wayland-install/* /usr/
	echo "install opencv ok..."
fi

# useless in zeos -- rocky - 20170613
if [ -f "./cv.tar.bz2" ] ;then
	echo "install opencv for wayland..."
	tar -xvjf cv.tar.bz2 -C /usr/
	echo "install opencv ok..."
fi


echo "install all ok......"

#write log
if [ ! -f "/etc/update.log" ] ; then
	echo "update" > /etc/update.log
fi
date >> /etc/update.log

echo "syncing..."
sync

exit 0

