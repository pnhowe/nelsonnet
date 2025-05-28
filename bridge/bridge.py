#!/usr/bin/env -S python3 -u

import serial
import socket
import pickle
import struct

from config import GRAPHITE_PORT, GRAPHITE_SERVER

RECIEVER_PORT = '/dev/ttyACM0'

def main():
  print( 'Starting....' )
  print( f'Opening "{RECIEVER_PORT}"...' )

  reciever = serial.Serial( RECIEVER_PORT, 9600, timeout=( 60 * 15 ) )  # default is 8n1

  print( f'Connecting to "{GRAPHITE_SERVER}":"{GRAPHITE_PORT}"...' )

  print( 'Looking for data...' )
  try:
    while True:
      try:
        line = reciever.readline().decode().strip()
      except Exception as e:
        print( 'Error Reading line:', e )
        continue

      if line[0] != '!' or line[-1] != '!':
        print( f'# {line}')
        continue

      print( 'reading value...' )
      try:
        (_, msglen, _) = line.split( '\t' )
        msglen = int( msglen )
      except ( IndexError, ValueError ):
        print( 'Bad Msg Len' )
        print( line )
        continue

      line = reciever.readline().decode().strip()

      if line[0] != '$' or line[-1] != '$' or len( line ) != msglen:
        print( f'# {line}')
        continue

      data_list = line.split( '\t' )
      data_list.pop(0)
      data_list.pop()

      try:
        ( node_name, rRSSI, send_counter, sync_counter ) = data_list.pop(0).split( ':' )
        rRSSI = int( rRSSI )
        send_counter = int( send_counter )
        sync_counter = int( sync_counter )
      except ( KeyError, ValueError ):
        print( 'Bad rRSSI/send_counter/sync_counter' )
        print( line )
        continue

      line = reciever.readline().decode().strip()
      if line[0] != '#' or line[-1] != '#':
        print( 'Bad lRSSI' )
        print( line )
        continue

      try:
        (_, lRSSI, _) = line.split( '\t' )
        lRSSI = int( lRSSI )
      except ( IndexError, ValueError ):
        print( 'Bad lRSSI value' )
        print( line )
        continue

      metric_list = []
      print( f'Node "{node_name}" with Remote RSSI "{rRSSI}", Local RSSI "{lRSSI}", Send Count "{send_counter}", Sync Count "{sync_counter}"' )
      metric_list.append( ( f'rssi.remote.{node_name}', ( -1, rRSSI ) ) )
      metric_list.append( ( f'rssi.local.{node_name}', ( -1, lRSSI ) ) )
      metric_list.append( ( f'send_counter.{node_name}', ( -1, send_counter ) ) )
      metric_list.append( ( f'sync_counter.{node_name}', ( -1, sync_counter ) ) )

      for data in data_list:
        try:
          ( metric, value ) = data.split( ':' )
          value = int( value )
        except ( IndexError, ValueError ):
          print( 'Bad Value' )
          continue

        print( f'Recieved "{metric}": "{value}"' )
        #TODO:  some metrics will need to be pre-processed, have a map that can be looked up via node_name and metric, and it will have a callable to tweek the value
        metric_list.append( ( f'data.{node_name}.{metric}', ( -1, value ) ) )

      graphite = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
      graphite.connect( ( GRAPHITE_SERVER, GRAPHITE_PORT ) )
      payload = pickle.dumps( metric_list, protocol=2 )
      header = struct.pack( '!L', len( payload ) )
      graphite.sendall( header + payload )
      graphite.close()

  except Exception as e:
    print( 'Got Exception', e )

  finally:
    reciever.close()

  print( 'Done' )


if __name__ == '__main__':
  main()
