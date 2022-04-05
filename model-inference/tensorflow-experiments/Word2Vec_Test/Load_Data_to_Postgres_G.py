import psycopg2
import numpy as np
import pandas as pd

vocab_size = 1009375

# connect to the Postgres
t_host = "localhost"
t_port = "5432"
t_dbname = "postgres"
t_user = "postgres"
t_pw = "postgres"
db_conn = psycopg2.connect(host=t_host, port=t_port, dbname=t_dbname, user=t_user, password=t_pw)
db_cursor = db_conn.cursor()

# create the table named text_G
try:
    db_cursor.execute("CREATE TABLE text_G(id int, array_data bytea);")
except psycopg2.Error as e:
    print(e)

# create the data which will be put into Postgres
A = np.random.randint(0, vocab_size-1, (1000000,))

# load the input to Postgres for one column for all the features
try:
    a_bytes = A.tobytes()
    # execute the INSERT statement
    db_cursor.execute("INSERT INTO text_G(id,array_data) " + "VALUES(%s,%s)",(0, a_bytes))
    # commit the changes to the database
    db_conn.commit()
    # close the communication with the PostgresQL database
    db_cursor.close()
except(Exception, psycopg2.DatabaseError) as error:
    print(error)
finally:
    if db_conn is not None:
        db_conn.close()