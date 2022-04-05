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

# create the table named text_M
try:
    db_cursor.execute("CREATE TABLE text_M(id int);")
    for i in range(400):
        db_cursor.execute("ALTER TABLE text_M ADD N" + str(i) + "Column bytea;")
except psycopg2.Error as e:
    print(e)

# create the data which will be put into Postgres
A =[]
for i in range(400):
    if (i == 399):
        A.append(np.random.rand(100, 2299))
    else:
        A.append(np.random.rand(100, 2524))

# load the input to Postgres for multiple columns
try:
    for j in range(400):
        this_array = A[j]
        for i in range(100):
            a = this_array[i]
            a_bytes = a.tobytes()
            # execute the INSERT statement
            if (j == 0):
                db_cursor.execute("INSERT INTO text_M (id,N"+ str(j) + "Column) " + "VALUES(%s,%s) ;",(i, a_bytes))
            else:
                db_cursor.execute("UPDATE text_M SET N" + str(j) + "Column = " + "%s where id = %s ;",(a_bytes,i))
            # commit the changes to the database
            db_conn.commit()
    # close the communication with the PostgresQL database
    db_cursor.close()
except(Exception, psycopg2.DatabaseError) as error:
    print(error)
finally:
    if db_conn is not None:
        db_conn.close()