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
query = "SELECT * FROM esp_data;"
cur = conn.cursor()
# Execute the query
cur.execute(query)
#cursor.execute("SELECT * FROM DB_table WHERE id = 1")
data = cur.fetchall()
for row in data:
    print(row)
cur.close()
conn.close()