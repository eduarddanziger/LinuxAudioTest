sudo -i; passwd
dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart
https://wslstorestorage.blob.core.windows.net/wslblob/wsl_update_x64.msi
wsl --set-default-version 2
wsl --install -d Ubuntu-24.04
https://apt.kitware.com/
sudo apt-get update
sudo apt-get install ninja-build
sudo apt update && sudo apt install libasound2-dev
sudo apt-get update && sudo apt-get upgrade -y
sudo apt install gdb

cd /home/ed/.vs/AudioTest/out/build/linux-debug/

CMake:
target_include_directories(AudioTest PRIVATE "/usr/include/alsa")
target_link_libraries(AudioTest PRIVATE asound)

sudo apt install net-tools
sudo apt install bind9-dnsutils

c:\Program Files (x86)\PulseAudio\etc\pulse\default.pa:
    load-module module-native-protocol-tcp auth-ip-acl=127.0.0.1;172.16.0.0/12
    load-module module-esound-protocol-tcp auth-ip-acl=127.0.0.1;172.16.0.0/12

cat /etc/resolv.conf | grep nameserver | awk '{print $2}'
export PULSE_SERVER=tcp:172.27.240.1
Better:
echo "export PULSE_SERVER=tcp:$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}')" >> ~/.bashrc


aplay -D pulse /usr/share/sounds/alsa/Front_Center.wav

aplay /usr/share/sounds/alsa/Front_Center.wav

To be able to debug:
1. Open the launch.vs.json File
    In Visual Studio, open your CMake project.
    Go to the Solution Explorer.
    Expand the CMake Targets View (if not visible, enable it via View > CMake Targets).
    Right-click on the target you want to debug (e.g., your executable) and select Add Debug Configuration.
    This will open or create the launch.vs.json file in the .vs folder of your project.

2. Modify the launch.vs.json File
    ...
      "debuggerConfiguration": "gdb",
      "args": [],
      "environment": [
        {
          "name": "PULSE_SERVER",
          "value": "tcp:172.30.96.1"
        }
      ]
      ...

