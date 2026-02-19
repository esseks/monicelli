VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  config.vm.box = "ubuntu/trusty64"
  config.vm.provision "shell", path: "vagrant/setup.sh"
  config.vm.synced_folder ".", "/opt/monicelli", owner: "vagrant", group: "vagrant"
end
