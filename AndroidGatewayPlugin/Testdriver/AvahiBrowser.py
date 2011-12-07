import dbus, gobject, avahi
from dbus import DBusException
from dbus.mainloop.glib import DBusGMainLoop

# Looks for iTunes shares

TYPE = '_ammogateway._tcp'

class AvahiBrowser:
  def service_resolved(self, *args):
    print 'Service resolved:'
    print '  name:', args[2]
    print '  address:', args[7]
    print '  port:', args[8]
    self.serverAddress = args[7]
    self.serverPort = args[8]
    self.mainLoop.quit()
  
  def print_error(self, *args):
      print 'error_handler'
      print args[0]
      
  def myhandler(self, interface, protocol, name, stype, domain, flags):
      print "Found service '%s' type '%s' domain '%s' " % (name, stype, domain)
  
      if flags & avahi.LOOKUP_RESULT_LOCAL:
              # local service, skip
              pass
  
      self.server.ResolveService(interface, protocol, name, stype, 
          domain, avahi.PROTO_INET, dbus.UInt32(0), 
          reply_handler=self.service_resolved, error_handler=self.print_error)
  
  def searchForServices(self):
    self.loop = DBusGMainLoop()
    
    self.bus = dbus.SystemBus(mainloop=self.loop)
    
    self.server = dbus.Interface( self.bus.get_object(avahi.DBUS_NAME, '/'),
            'org.freedesktop.Avahi.Server')
    
    self.sbrowser = dbus.Interface(self.bus.get_object(avahi.DBUS_NAME,
            self.server.ServiceBrowserNew(avahi.IF_UNSPEC,
                avahi.PROTO_INET, TYPE, 'local', dbus.UInt32(0))),
            avahi.DBUS_INTERFACE_SERVICE_BROWSER)
    
    self.sbrowser.connect_to_signal("ItemNew", self.myhandler)
    
    self.mainLoop = gobject.MainLoop()
    print "Looking for a gateway via Avahi..."
    self.mainLoop.run()
    
    return (self.serverAddress, int(self.serverPort))
