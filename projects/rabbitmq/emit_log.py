import sys
import pika

message = ' '.join(sys.argv[1:]) or "info: Hello World!"

connection = pika.BlockingConnection(pika.ConnectionParameters('localhost'))
channel = connection.channel()
channel.exchange_declare(exchange='logs', type='fanout', nowait=True)
#channel.queue_declare(queue='hello')
#channel.basic_publish(exchange='', routing_key='hello', body=message)
channel.basic_publish(exchange='logs', routing_key='', body=message)
connection.close()
