
apt -y remove ansible
apt -y --purge autoremove

apt update
apt -y upgrade
apt -y install software-properties-common
apt-add-repository ppa:ansible/ansible
apt -y install ansible

