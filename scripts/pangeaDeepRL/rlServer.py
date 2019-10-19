#!/usr/bin/env python


#JiaNote: this file implements a TCP/IP server that repeatedly receives prediction request containing current state and last reward in a JSON format, and replies with the predicted action.

import socket
import base64
import urllib
import sys
import os
import json
import random
import numpy as np
import tensorflow as tf
import time
import a3c
import struct
import datetime

#for each data, we only consider at most 2 candidate
K = 2

#action space dimension: equivalent to the number of candidates plus 1 for no partition
A_DIM = K + 1

#state space dimension
S_DIM = 4 * K + 7

#actor network learning rate
ACTOR_LR_RATE = 0.0001

#critic network learning rate
CRITIC_LR_RATE = 0.001

#log file
LOG_FILE = "./rlLog"
SUMMARY_DIR = "./results"
MODEL_DIR = "."
#default lambda
DEFAULT_LAMBDA = 0

#size of a training batch
TRAIN_SEQ_LEN = 96

#size of gradient batch
GRADIENT_BATCH_SIZE = 16

#number of epochs
MODEL_SAVE_INTERVAL = 96

#saved model for restoring
#NN_MODEL = None
NN_MODEL = './nn_model_ep_864.ckpt'

#random seed
RANDOM_SEED = 42
RAND_RANGE = 1000

MAX_EPOCH = 65535


def run(port=8333, log_file_path=LOG_FILE):

    np.random.seed(RANDOM_SEED)
    with tf.Session() as sess, open(log_file_path, 'wb') as log_file:

        actor = a3c.ActorNetwork(sess, state_dim=S_DIM, action_dim=A_DIM,  
                                 learning_rate=ACTOR_LR_RATE)
        critic = a3c.CriticNetwork(sess, state_dim=S_DIM, 
                                   learning_rate=CRITIC_LR_RATE)

        summary_ops, summary_vars = a3c.build_summaries()
        sess.run(tf.initialize_all_variables())
        writer = tf.summary.FileWriter(SUMMARY_DIR, sess.graph)
        saver = tf.train.Saver() # save neural net parameters

        #restore neural net parameters
        nn_model = NN_MODEL
        if nn_model is not None: # nn_model is the path to file
            saver.restore(sess, nn_model)
            print("Model restored.")

        init_action = np.zeros(A_DIM)
        #by default we simply use the first lambda
        init_action[DEFAULT_LAMBDA] = 0

        s_batch = [np.zeros(S_DIM)]
        a_batch = [init_action]
        r_batch = []
        entropy_record = [] #this is for training
        
        actor_gradient_batch = [] #this is for training
        critic_gradient_batch = [] #this is for training


        last_lambda = DEFAULT_LAMBDA  
        epoch = 0
        end_of_training = False
        # Create a TCP/IP socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # Bind the socket to the port
        server_address = ('localhost', port)
        print >>sys.stderr, 'starting up on %s port %s' % server_address 
        sock.bind(server_address)

        # Listen for incoming connections
        sock.listen(5)
        count = 0


        while True:
            # Wait for a connection
            print >>sys.stderr, 'waiting for a connection'
            connection, addr = sock.accept()
            print 'Connected with ' + addr[0] + ':' + str(addr[1])

            # Receive the json file
            # json file format:
            # 'reward': float
            # 'state': array = '{"state": ["1", "3", "4", ...]}'
            #numBytes = sys.getsizeof(int)
            #print ("size to receive: " + str(numBytes))
            size=connection.recv(4)
            size=struct.unpack('!i', size)[0]
            print >>sys.stderr, 'received "%s"' % size
            data = connection.recv(size)
            jsonData = json.loads(data)
            print jsonData

            #to receive reward
            reward = float(jsonData['reward'])
            if (count > 0):
                r_batch.append(reward)
            else:
                r_batch.append(0.0)
           
            count = count + 1
            #to receive state
            stateArray = jsonData['state']
            state = np.array(stateArray)
            print(state)
            #to compute action
            action_prob = actor.predict(np.reshape(state, (1, S_DIM)))
            print ("action_prob: ")
            print (action_prob)
            action_cumsum = np.cumsum(action_prob)
            print ("action_cumsum: ")
            print (action_cumsum)
            print ("comparison: ")
            print (action_cumsum > np.random.randint(1, RAND_RANGE) / float(RAND_RANGE)) 
            selectedLambda = action_prob.argmax()
            #selectedLambda = (action_cumsum > np.random.randint(1, RAND_RANGE) / float(RAND_RANGE)).argmax()
            print >>sys.stderr, 'selectedLambda "%s"' % selectedLambda
            #to update entropy
            entropy_record.append(a3c.compute_entropy(action_prob[0])) #TODO

            #to update and apply gradient 
            if len(r_batch) >= TRAIN_SEQ_LEN:
                    actor_gradient, critic_gradient, td_batch = \
                       a3c.compute_gradients(s_batch=np.stack(s_batch[1:], axis=0),
                                            a_batch=np.vstack(a_batch[1:]),
                                            r_batch=np.vstack(r_batch[1:]),
                                            terminal=end_of_training, actor=actor, critic=critic)
                    td_loss = np.mean(td_batch)    

                    print ("td_loss: ")
                    print (td_loss)
                    print ("actor_gradient: ")
                    print (actor_gradient)
                    print ("critic_gradient: ")
                    print (critic_gradient)

                    actor_gradient_batch.append(actor_gradient)
                    critic_gradient_batch.append(critic_gradient)

                    entropy_record = []
                    print ("len(actor_gradient_batch) = ")
                    print len(actor_gradient_batch)
                    if len(actor_gradient_batch) >= GRADIENT_BATCH_SIZE:
                        print ("GRADIENT_BATCH_SIZE reached")
                        assert len(actor_gradient_batch) == len(critic_gradient_batch)
                        for i in xrange(len(actor_gradient_batch)):
                            print ("###################" + str(i) + "###################")
                            print (actor_gradient_batch[i])
                            print (critic_gradient_batch[i])
                            actor.apply_gradients(actor_gradient_batch[i])
                            critic.apply_gradients(critic_gradient_batch[i])

                        actor_gradient_batch = []
                        critic_gradient_batch = []

                        avg_reward = np.mean(r_batch)
                        summary_str = sess.run(summary_ops, feed_dict={
                            summary_vars[0]: td_loss,
                            summary_vars[1]: avg_reward
                        })
                                     
                        writer.add_summary(summary_str, epoch)
                        writer.flush()
                        log_file.write(str(datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')) + '\t' +
                                       str(epoch) + '\t' +
                                       str(avg_reward) + '\t' +
                                       str(td_loss) + '\n')
                        log_file.flush()

                        epoch += 1
                        if epoch % MODEL_SAVE_INTERVAL == 0:
                            # save the neural net parameters to disk.
                            save_path = saver.save(sess, "./nn_model_ep_" + str(epoch) + ".ckpt")
                            print("Model saved in file: %s" % save_path)

                        if epoch == MAX_EPOCH:
                            end_of_training = True
                         

                    del s_batch[:]
                    del a_batch[:]
                    del r_batch[:]

            s_batch.append(state)
            action_vec = np.zeros(A_DIM)
            action_vec[selectedLambda] = 1
            a_batch.append(action_vec)

            #to send back action
            print >>sys.stderr, 'sending data back to the client'
            connection.sendall(struct.pack('!i',selectedLambda))
            last_lambda = selectedLambda
            connection.close()

        sock.close()    


def main():
    if len(sys.argv) == 2:
        trace_file = sys.argv[1]
        run(log_file_path=trace_file)
    else:
        run()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print "Keyborad interrupted."
        try: 
            sys.exit(0)
        except SystemExit:
            os._exit(0) 
