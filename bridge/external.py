#!/usr/bin/env python3 -u

import time
import socket
import pickle
import struct
import requests
from dateutil import parser

from config import GRAPHITE_PORT, GRAPHITE_SERVER


def main():
  print( 'Starting....' )

  print( f'Connecting to "{GRAPHITE_SERVER}":"{GRAPHITE_PORT}"...' )

  try:
    while True:
      try:
        observed = requests.get( 'https://api.water.noaa.gov/nwps/v1/gauges/COAU1/stageflow/observed' )
      except Exception as e:
        print( "Got Exception getting data", e, e.__class__ )
        time.sleep( 15 )
        continue

      data = observed.json()

      metric_list = []

      for item in data[ 'data' ][-10:]:  # we we are geting 30 min at a time, the data is every 5 min, so 6 plus a few to overlap, just in case
        metric_list.append( ( f'coalcreek.level', ( parser.parse( item['validTime'] ).timestamp(), item[ 'primary' ] ) ) )

      print( metric_list )
      graphite = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
      graphite.connect( ( GRAPHITE_SERVER, GRAPHITE_PORT ) )
      payload = pickle.dumps( metric_list, protocol=2 )
      header = struct.pack( '!L', len( payload ) )
      graphite.sendall( header + payload )
      graphite.close()

      time.sleep( 30 * 60 ) # check every 30 min

  except Exception as e:
    print( "Got Exception", e, e.__class__ )

  print( "Done" )


if __name__ == '__main__':
  main()
