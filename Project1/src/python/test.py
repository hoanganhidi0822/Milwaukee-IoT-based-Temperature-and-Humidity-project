import requests
import json
 
url = "http://10.147.37.59:5001/api/esp_data/firm_ver?key=XXV7lnIse9q4YGA11pXA&code=30BB7BBD4D74"
 
payload = json.dumps({
  "firm_ver": "1.12"
})
headers = {
  'Content-Type': 'application/json'
}
 
response = requests.request("PUT", url, headers=headers, data=payload)
 
print(response.text)