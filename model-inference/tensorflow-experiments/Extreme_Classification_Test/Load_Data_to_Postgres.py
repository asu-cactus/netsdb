import psycopg2
import numpy as np
import pandas as pd

# connect to the Postgres
t_host = "localhost"
t_port = "5432"
t_dbname = "postgres"
t_user = "postgres"
t_pw = "postgres"
db_conn = psycopg2.connect(host=t_host, port=t_port, dbname=t_dbname, user=t_user, password=t_pw)
db_cursor = db_conn.cursor()

# create the table named extreme
try:
    db_cursor.execute("CREATE TABLE extreme(id int, array_data bytea);")
except psycopg2.Error as e:
    print(e)

# create the data which will be put into Postgres
A = np.random.rand(1000, 597540)

# load the input to Postgres for one column for all the features
try:
    for i in range(1000):
        a = A[i]
        a_bytes = a.tobytes()
        # execute the INSERT statement
        db_cursor.execute("INSERT INTO extreme(id,array_data) " +
                    "VALUES(%s,%s)",(i, a_bytes))
        # commit the changes to the database
        db_conn.commit()
    # close the communication with the PostgresQL database
    db_cursor.close()
except(Exception, psycopg2.DatabaseError) as error:
    print(error)
finally:
    if db_conn is not None:
        db_conn.close()