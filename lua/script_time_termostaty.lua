adres = '127.0.0.1:8888'
salonIDX = 8
kuchniaIDX = 7
bableIDX = 6
hubertIDX = 5

function czasokres(czas1, czas2) --czas w formacie hh:mm:ss, czas1 < czas2
	t0 = os.time()
	
	rok = os.date("%Y")
   miesiac = os.date("%m")
   dzien = os.date("%d")
   
	g1 = string.sub(czas1, 1, 2)
   m1 = string.sub(czas1, 4, 5)
   s1 = string.sub(czas1, 7, 8)
	g2 = string.sub(czas2, 1, 2)
   m2 = string.sub(czas2, 4, 5)
   s2 = string.sub(czas2, 7, 8)
	
	t1 = os.time{year=rok, month=miesiac, day=dzien, hour=g1, min=m1, sec=s1}
	t2 = os.time{year=rok, month=miesiac, day=dzien, hour=g2, min=m2, sec=s2}
	
	if (os.difftime(t0, t1) >= 0) and (os.difftime(t0, t2) < 0) then
		return 1
	else
		return 0
	end
end

-- trzeba pamiętać, by sprawdzając czy jesteśmy między wieczorem a ranem sprawdzać
-- 2 okresy: od wieczora do północy i od północy do rana, związane jest to z tym, że funkcja czasokres
-- korzysta z z os.difftime, która bierze pod uwagę także datę

-- wymagane zmienne użytkownika: tempNoc, tempDzien, tempNocBable

function termostatSetPoint(termostat, termostatIDX, rano, tempRano, poludnie, tempPoludnie, wieczor, tempWieczor)
	--print(termostat)
	--print(czasokres('13:25:00', '13:50:00'))
	ustawionaTemp=tonumber(otherdevices_svalues[termostat]) --svalues zwraca wynik z dwoma cyframi po przecinku, tonumber obcina
	--print(ustawionaTemp)
	--print(tempRano)
	
	if (czasokres(rano, poludnie) == 1) and (ustawionaTemp ~= tempRano) then
		print('Ustaw temperaturę na tempRano') -- trzeba zrobić by funkcja zwracała string, który będzie można przekazać do openURL przez commandArray
		json = adres..'/json.htm?type=command&param=udevice&idx='..termostatIDX..'&nvalue=0&svalue='..tempRano
		zmien = true
	elseif (czasokres(poludnie, wieczor) == 1) and (ustawionaTemp ~= tempPoludnie) then
		--print('Ustaw temperaturę na tempPoludnie')
		--print('Temperatura ustawiona - '..ustawionaTemp)
		--print('Temperatura południe - '..tempPoludnie)
		json = adres..'/json.htm?type=command'..'&'..'param=udevice&idx='..termostatIDX..'&nvalue=0&svalue='..tempPoludnie
		zmien = true
	elseif (czasokres(wieczor, '23:59:59') == 1) and (ustawionaTemp ~= tempWieczor) then
		print('Ustaw temperaturę na tempWieczor')
		json = adres..'/json.htm?type=command&param=udevice&idx='..termostatIDX..'&nvalue=0&svalue='..tempWieczor
		zmien = true
	elseif (czasokres('00:00:00', rano) == 1) and (ustawionaTemp ~= tempWieczor) then
		print('Ustaw temperaturę na tempWieczor')
		json = adres..'/json.htm?type=command&param=udevice&idx='..termostatIDX..'&nvalue=0&svalue='..tempWieczor
		zmien = true
	else
		zmien = false
	end
	return zmien, json
end



commandArray = {}

	if (devicechanged['Testowy']) then
		print('---------- Wciśnięto Testowy ------------')
		--print(otherdevices_svalues['Salon'])
		zmien, json = termostatSetPoint('Salon', 8, '06:00', uservariables["tempNoc"], '16:00', uservariables["tempDzien"], '20:00', uservariables["tempNoc"])
		if zmien then commandArray['OpenURL']=json end
		zmien, json = termostatSetPoint('Kuchnia', 7, '06:00', uservariables["tempNoc"], '16:00', uservariables["tempDzien"], '20:00', uservariables["tempNoc"])
		if zmien then commandArray['OpenURL']=json end
		zmien, json = termostatSetPoint('Bąble', 6, '06:00', uservariables["tempNoc"], '16:00', uservariables["tempDzien"], '20:00', uservariables["tempNoc"])
		if zmien then commandArray['OpenURL']=json end
		zmien, json = termostatSetPoint('Hubert', 5, '06:00', uservariables["tempNoc"], '16:00', uservariables["tempDzien"], '20:00', uservariables["tempNoc"])
		if zmien then commandArray['OpenURL']=json end
		print('-----------------------------------------')
	end

--[[	
	if (devicechanged['Wyłącz ogrzewanie'] == 'Off') then
		print('-------------')
		commandArray[1]={['OpenURL'] = '127.0.0.1:8080/json.htm?type=command&param=udevice&idx=8&nvalue=0&svalue=0'} --termostat Bąble
		commandArray[2]={['OpenURL'] = '127.0.0.1:8080/json.htm?type=command&param=udevice&idx=13&nvalue=0&svalue=5'} --termostat Salon
		commandArray[3]={['OpenURL'] = '127.0.0.1:8080/json.htm?type=command&param=udevice&idx=14&nvalue=0&svalue=0'} --termostat Kuchnia
		commandArray[4]={['OpenURL'] = '127.0.0.1:8080/json.htm?type=command&param=udevice&idx=15&nvalue=0&svalue=0'} --termostat Hubert
		--commandArray['Wyłącz ogrzewanie'] = 'On'
	end

	if (devicechanged['Bąble']) then
		print('Zmieniono ustawienia termostatu')
		print(tonumber(otherdevices_svalues['Salon']))
		--test('salon')
	end
]]--
	
--	print(devicechanged['Bąble w domu'] .. '----------')
--[[	
	if (devicechanged['Okno Bąble'] == 'On') then
		print('Zmień na tempOkno u Bąbli')
	elseif (devicechanged['Okno Bąble'] == 'Off') then
		print('Zmień na tempBable u Bąbli')
	end
	
	if (devicechanged['Nie ma nas'] == 'On') then
		print('Nie ma nas on')
	elseif (devicechanged['Nie ma nas'] == 'Off') then
		print ('Nie ma nas off')
	end
--]]

-- info o commandArray: 	
	

return commandArray
