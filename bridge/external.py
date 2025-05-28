#!/usr/bin/env -S python3 -u

import time
import socket
import pickle
import struct
import requests
from dateutil import parser

from config import GRAPHITE_PORT, GRAPHITE_SERVER

# in minutes
COAL_CREEK_INTERVAL = 30
UPPER_DITCH = 1

def send( metric_list ):
  print( metric_list )
  graphite = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
  graphite.connect( ( GRAPHITE_SERVER, GRAPHITE_PORT ) )
  payload = pickle.dumps( metric_list, protocol=2 )
  header = struct.pack( '!L', len( payload ) )
  graphite.sendall( header + payload )
  graphite.close()



def main():
  print( 'Starting....' )

  print( f'Connecting to "{GRAPHITE_SERVER}":"{GRAPHITE_PORT}"...' )

  counter = 0

  try:
    while True:
      if counter % COAL_CREEK_INTERVAL == 0:
        try:
          req = requests.get( 'https://api.water.noaa.gov/nwps/v1/gauges/COAU1/stageflow/observed' )
          data = req.json()
          metric_list = []
          for item in data[ 'data' ][-10:]:  # we we are geting 30 min at a time, the data is every 5 min, so 6 plus a few to overlap, just in case
            metric_list.append( ( f'coalcreek.level', ( parser.parse( item['validTime'] ).timestamp(), item[ 'primary' ] ) ) )
          send( metric_list )

        except Exception as e:
          print( 'Exception getting Coal Creek Data', e, e.__class__ )

      if counter % UPPER_DITCH == 0:
        try:
          req = requests.get( 'http://192.168.16.51/raw' )
          data = req.json()
          metric_list = []
          metric_list.append( ( 'data.upper_ditch.level', ( -1, float( data ) ) ) )
          send( metric_list )

        except Exception as e:
          print( 'Exception getting Upper Ditch Data', e, e.__class__ )

      counter += 1
      time.sleep( 60 ) # Wait 1 min

  except Exception as e:
    print( "Got Exception", e, e.__class__ )

  print( "Done" )


if __name__ == '__main__':
  main()
