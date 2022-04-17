import psycopg2
import numpy as np
import torch
import time

def get_db_connection():
    # connect to postgresql database
    t_host = "localhost"
    t_port = "5432"
    t_dbname = "postgres"
    t_user = "postgres"
    t_pw = "postgres"
    return psycopg2.connect(host=t_host, port=t_port, dbname=t_dbname, user=t_user, password=t_pw)


def create_tables(db_cursor):
    try:
        db_cursor.execute("DROP TABLE IF EXISTS images;")
        db_cursor.execute("DROP TABLE IF EXISTS kernels;")
        db_cursor.execute("CREATE TABLE IF NOT EXISTS images(id int, array_data bytea);")
        db_cursor.execute("CREATE TABLE IF NOT EXISTS kernels(id int, array_data bytea);")
    except psycopg2.Error as e:
        print(e)

def load_input_to_db(db_connection, input_dimensions, iterations):
    db_cursor = db_connection.cursor()
    for id in range (0,iterations):
        img = np.random.rand(*input_dimensions).astype(np.float32)
        img_bytes = img.tobytes()
        db_cursor.execute("INSERT INTO images(id,array_data) " + "VALUES(%s,%s)",(id, img_bytes))
    
    # commit the changes to the database
    db_connection.commit()
    db_cursor.close()
    
def load_kernel_to_db(db_connection, kernel_dimensions, kernel_id):
    db_cursor = db_connection.cursor()
    kernel = np.random.rand(*kernel_dimensions).astype('f')
    kernel_bytes = kernel.tobytes()
    db_cursor.execute("INSERT INTO kernels(id,array_data) " + "VALUES(%s,%s)",(kernel_id, kernel_bytes))
    print (kernel.shape)

    # commit the changes to the database
    db_connection.commit()
    db_cursor.close()

def load_input_to_file_torch(input_file_path, input_dimensions, iterations):
    start = time.time()
    for id in range (0,iterations):
        img = torch.randn(*input_dimensions, dtype=torch.float32)
        # img = np.random.rand(*input_dimensions)
        torch.save(img, input_file_path + str(id) + '.pt')

    end = time.time()
    print ("Time for saving input to file", end - start)

def load_input_to_file(input_file_path, input_dimensions, iterations):
    for id in range (0,iterations):
        img = np.random.rand(*input_dimensions).astype('f')
        np.save(input_file_path + str(id), img)

def load_kernel_to_file_torch(kernel_file_path, kernel_dimensions):
    img = torch.randn(*kernel_dimensions)
    torch.save(img, kernel_file_path + '.pt')

def load_kernel_to_file(kernel_file_path, kernel_dimensions):
    kernel = np.random.rand(*kernel_dimensions).astype('f')
    np.save(kernel_file_path, kernel)

def _read_kernel_from_db(db_cursor, kernel_id, kernel_dimensions):
    db_cursor.execute(""" SELECT array_data FROM kernels WHERE id = %s """,(kernel_id,))
    blob = db_cursor.fetchone()
    filter = np.frombuffer(blob[0])
    filter = filter.reshape(*kernel_dimensions)
    return filter

def read_kernel_data(load_data_from_file, kernel_file_path, db_cursor, kernel_id, kernel_dimensions):
    if load_data_from_file:
        return np.load(kernel_file_path + '.npy')
    else:
        return _read_kernel_from_db(db_cursor, kernel_id, kernel_dimensions)

def read_input_from_db(db_cursor, id, input_dimensions):
    db_cursor.execute(""" SELECT array_data FROM images WHERE id = %s """,(id,))
    blob = db_cursor.fetchone()
    input = np.frombuffer(blob[0], dtype=np.float32)
    input = input.reshape(*input_dimensions)
    return input

def read_input_torch_batch_from_db(db_cursor, startId, endId, input_dimensions):
    db_cursor.execute(f"SELECT array_data FROM images WHERE id >= {startId} and id < {endId}")
    rows = db_cursor.fetchall()
    data = []
    for row in rows:
        row = torch.frombuffer(row[0], dtype=torch.float32)
        row = row.reshape(*input_dimensions)
        data.append(row)
    return data

def read_input_tensorflow_batch_from_db(db_cursor, startId, endId, input_dimensions):
    db_cursor.execute(f"SELECT array_data FROM images WHERE id >= {startId} and id < {endId}")
    rows = db_cursor.fetchall()
    data = []
    for row in rows:
        row = np.frombuffer(row[0], dtype=np.float32)
        row = row.reshape(*input_dimensions)
        data.append(row)
    return data