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

# create the table named extreme_M
try:
    db_cursor.execute("CREATE TABLE extreme_M(id int);")
    for i in range(299):
        db_cursor.execute("ALTER TABLE extreme_M ADD N" + str(i) + "Column bytea;")
except psycopg2.Error as e:
    print(e)

# create the data which will be put into Postgres
A =[]
for i in range(299):
    if (i == 298):
        A.append(np.random.rand(1000, 1540))
    else:
        A.append(np.random.rand(1000, 2000))

# load the input to Postgres for multiple columns
try:
    for j in range(299):
        this_array = A[j]
        for i in range(1000):
            a = this_array[i]
            a_bytes = a.tobytes()
            # execute the INSERT statement
            if (j == 0):
                db_cursor.execute("INSERT INTO extreme_M (id,N"+ str(j) + "Column) " + "VALUES(%s,%s) ;",(i, a_bytes))
            else:
                db_cursor.execute("UPDATE extreme_M SET N" + str(j) + "Column = " + "%s where id = %s ;",(a_bytes,i))
            # commit the changes to the database
            db_conn.commit()
    # close the communication with the PostgresQL database
    db_cursor.close()
except(Exception, psycopg2.DatabaseError) as error:
    print(error)
finally:
    if db_conn is not None:
        db_conn.close()