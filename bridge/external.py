#!/usr/bin/env python3


import socket
import pickle
import struct

GRAPHITE_PORT = 2004
GRAPHITE_SERVER = '127.17.0.1'

def main():
  print( 'Starting....' )
  
  print( f'Connecting to "{GRAPHITE_SERVER}":"{GRAPHITE_PORT}"...' )
  
  try:
    while True:
      

      metric_list = []

      
      metric_list.append( ( f'{node_name}.{metric}', ( -1, value ) ) )
      
      print( metric_list )
      graphite = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
      graphite.connect( ( GRAPHITE_SERVER, GRAPHITE_PORT ) )
      payload = pickle.dumps( metric_list, protocol=2 )
      header = struct.pack( '!L', len( payload ) )
      graphite.sendall( header + payload )
      graphite.close()
  
  except Exception as e:
    print( "Got Exception", e )
  
  finally:
    reciever.close()
  
  print( "Done" )
  

if __name__ == '__main__':
  main()
