import psycopg2

DB_NAME = "ESP32"
DB_HOST = "10.147.37.59"
DB_USER = "esp32"
DB_PASS = "eGghHQpUxXzXS7"
DB_PORT = "5432"

conn = psycopg2.connect(database=DB_NAME,
                        host=DB_HOST,
                        user=DB_USER,
                        password=DB_PASS,
                        port=DB_PORT)
print(conn)

cur = conn.cursor()
insert_query = """
    INSERT INTO esp_data(org, dept, room)
    VALUES (%s, %s, %s);
"""

# Define the data to be inserted
data_to_insert = ('value1', 'value2', 'value3')

# Execute the insert query
cur.execute(insert_query, data_to_insert)

# Commit the transaction
conn.commit()

# Optionally, check the number of rows inserted
rows_inserted = cur.rowcount
print(f"Number of rows inserted: {rows_inserted}")

# Close the cursor and connection
cur.close()
conn.close()