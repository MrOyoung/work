#!/bin/sh

service_file="log_mgr"

cp ${service_file} /usr/bin
chmod +x /usr/bin/${service_file}

echo "install service ok ..."


script_file="syslog_mgr.service"
cp ${script_file} /lib/systemd/system 
cp noempty /usr/bin
chmod +x /usr/bin/noempty

systemctl enable log_mgr.service

echo "install syslog_mgr.service ok ..."

sync
exit 0
