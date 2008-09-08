# Common methods for determining the system properties, eg. the operating
# system, local ip address etc.

import os
import socket

def osIsWin():
  if os.name == 'nt':
    return True
  else:
    return False


def osIsNix():
  if os.name == 'posix':
    return True
  else:
    return False

  
def osIsVista():
  home = os.getenv('USERPROFILE')
  appdata = os.path.join(home, 'appdata')
  if os.path.exists(appdata):
    return True
  else:
    return False


def osIsMac():
  apps = '/Applications'
  if os.path.exists(apps):
    return True
  else:
    return False
    
    
def IsIpValid(ip) :
  """ Checks if a given IP address is a valid one. """
  octets = ip.split('.')
  if len(octets) != 4:
    return False

  # Check that all elements are valid octet digits.
  for octet in octets:
    if not (octet.isdigit() and int(octet) < 255):
      return False

  return True


# When a Windows Mobile device is connected to the test server using 
# ActiveSync over USB, the ActiveSync creates a logical network interface 
# between the mobile device and the host. If we drive the mobile device 
# to access the host machine, through the IP address assigned to the host for
# this interface, the connection is extremely slow.
#
# The class below provides helper methods to get the IP address assigned to the
# physical network interface card so that the mobile device can access the host
# server faster, hence improving the test execution latency. 
class WindowsNetworkHelper:

  @staticmethod
  def __IsWindowsMobileDevice(interface):
    """ Checks if the network interface is a Windows Mobile Based device. """
    
    device_name = interface.Caption
    if (device_name.find('Windows Mobile-Based Device') > -1):
      return True
    return False

  @staticmethod
  def __WinGetLocalIpAddress():
    """ Gets ip address of the physical network interface on a Windows host. """
    import wmi
      
    ip = "...."

    interfaces = []
    c = wmi.WMI()
    for interface in c.Win32_NetworkAdapterConfiguration(IPEnabled=1):
      # TODO(baran): There seems to be no easy way to detect that a network
      # interface is a physical one. This is a workaround to at least eliminate
      # the ones that we know are logical interfaces created by ActiveSync.
      # Replace this with a solution which can detect the physical interfaces.
      if not WindowsNetworkHelper.__IsWindowsMobileDevice(interface):
        interfaces.append(interface)

    # Get the interface with the lowest index
    if len(interfaces) > 0:
      interface_to_use = interfaces[0]
      index = interfaces[0].Index
      for interface in interfaces:
        if interface.Index < index:
          index = interface.Index
          interface_to_use = interface

      # Return the first valid ip address of this interface
      if not interface_to_use.IPAddress == None :
        ip = interface_to_use.IPAddress[0]

    return ip

  @staticmethod
  def GetLocalIp():
    """ Returns the local IP address by trying to get it first from
    the network interface configuration, and then failing over to
    hostname-to-ip resolution.
    """
    
    ip = "...."
    
    # First try and get the ip from the network interface cards.
    if osIsWin():
      ip = WindowsNetworkHelper.__WinGetLocalIpAddress()
      if IsIpValid(ip):
        return ip

    # If we can't get any ip addresses from the network interface cards, then 
    # try and get the ip from the hostname.
    ip = socket.gethostbyname(socket.gethostname())
    if IsIpValid(ip):
      return ip
    else:
      raise ValueError("Unable to retrieve an IP address for this host.")
