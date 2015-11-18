-- wymagane zmienne użytkownika: tempNoc, tempDzien, tempNocBable, tempNieMaNas
adres = '127.0.0.1:8080' --adres Domoticza
opoznienieOkna = 10 -- po jakim czasie w sekundach od zamknięcia okna ma powrócić ustawienie temperatury

tNoc = uservariables["tempNoc"]
tDzien = uservariables["tempDzien"]
tNocBable = uservariables["tempNocBable"]
tNiemanas = uservariables["tempNieMaNas"]

-- Warunek ustawiający temperaturę gdy nikogo nie ma w domu.
if (otherdevices["Nie ma nas"] == 'On') then
	tNoc = tNiemanas
	tDzien = tNiemanas
	tNocBable = tNiemanas
end

function czyWeekend()
	if ((os.date("%w") ~=0) and (os.date("%w") ~= 6)) then return false
	else return true
	end
end

if czyWeekend() then
	-- weekendowe czasy:
	czasSalon = {"07:00", "16:30", "21:00"}
	czasKuchnia = {"08:00", "16:30", "21:00"}
	czasBable = {"06:00", "16:30", "19:30"}
	czasHubert = {"06:00", "16:30", "21:00"}
else
	-- pracujące czasy:
	czasSalon = {"08:00", "16:30", "21:00"}
	czasKuchnia = {"08:00", "16:30", "21:00"}
	czasBable = {"08:00", "16:30", "19:30"}
	czasHubert = {"09:00", "16:30", "21:00"}
end







--[[
czasRanoSalon = "07:00"
czasPoludnieSalon = "16:30"
czasWieczorSalon = "

czasRanoKuchnia
czasPoludnieKuchnia
czasWieczorKuchnia

czasRanoBable
czasPoludnieBable
czasWieczorBable

czasRanoHubert
czasPoludnieHubert
czasWieczorHubert
--]]

function roznicaCzasu(czas)
   r = string.sub(czas, 1, 4)
   m = string.sub(czas, 6, 7)
   d = string.sub(czas, 9, 10)
   g = string.sub(czas, 12, 13)
   mi = string.sub(czas, 15, 16)
   s = string.sub(czas, 18, 19)
   t1 = os.time()
   t2 = os.time{year=r, month=m, day=d, hour=g, min=mi, sec=s}
   roznica = os.difftime (t1, t2)
   return roznica
end

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

-- funkcja wprowadza opóźnienie w zamknięciu okna
function oknoZamkniete(okno, opoznienie)
	--print(roznicaCzasu(otherdevices_lastupdate[okno]))
	if (otherdevices[okno] == 'On') then return false 
	elseif ((otherdevices[okno] == 'Off') and (roznicaCzasu(otherdevices_lastupdate[okno]) > opoznienie)) then return true
	else return false
	end
end

-- trzeba pamiętać, by sprawdzając czy jesteśmy między wieczorem a ranem sprawdzać
-- 2 okresy: od wieczora do północy i od północy do rana, związane jest to z tym, że funkcja czasokres
-- korzysta z z os.difftime, która bierze pod uwagę także datę



function termostatSetPoint(termostat, termostatIDX, rano, tempRano, poludnie, tempPoludnie, wieczor, tempWieczor, okno, tempOkno, wylacz)
	ustawionaTemp=tonumber(otherdevices_svalues[termostat]) --svalues zwraca wynik z dwoma cyframi po przecinku, tonumber obcina
	if (not wylacz) then
		if (oknoZamkniete(okno, opoznienieOkna)) then
			if (czasokres(rano, poludnie) == 1) and (ustawionaTemp ~= tempRano) then
				--print('Ustaw temperaturę na tempRano') -- trzeba zrobić by funkcja zwracała string, który będzie można przekazać do openURL przez commandArray
				json = adres..'/json.htm?type=command&param=udevice&idx='..termostatIDX..'&nvalue=0&svalue='..tempRano
				zmien = true
			elseif (czasokres(poludnie, wieczor) == 1) and (ustawionaTemp ~= tempPoludnie) then
				json = adres..'/json.htm?type=command'..'&'..'param=udevice&idx='..termostatIDX..'&nvalue=0&svalue='..tempPoludnie
				zmien = true
			elseif (czasokres(wieczor, '23:59:59') == 1) and (ustawionaTemp ~= tempWieczor) then
				--print('Ustaw temperaturę na tempWieczor')
				json = adres..'/json.htm?type=command&param=udevice&idx='..termostatIDX..'&nvalue=0&svalue='..tempWieczor
				zmien = true
			elseif (czasokres('00:00:00', rano) == 1) and (ustawionaTemp ~= tempWieczor) then
				--print('Ustaw temperaturę na tempWieczor')
				json = adres..'/json.htm?type=command&param=udevice&idx='..termostatIDX..'&nvalue=0&svalue='..tempWieczor
				zmien = true
			else
				zmien = false
			end
		elseif (ustawionaTemp ~= tempOkno) then
			json = adres..'/json.htm?type=command&param=udevice&idx='..termostatIDX..'&nvalue=0&svalue='..tempOkno
			zmien = true
		else 
			zmien = false
		end
	elseif (ustawionaTemp ~= tempOkno) then
		json = adres..'/json.htm?type=command&param=udevice&idx='..termostatIDX..'&nvalue=0&svalue='..tempOkno
		zmien = true
	else
		zmien = false
	end
	return zmien, json
end

if (otherdevices["Wyłącz ogrzewanie"] == 'On') then
	wyl = true
else
	wyl = false
end


commandArray = {}

--[[ - przeniesione na górę
-- Warunek ustawiający temperaturę gdy nikogo nie ma w domu.
	if (otherdevices["Nie ma nas"] == 'On') then
		tNoc = tNiemanas
		tDzien = tNiemanas
		tNocBable = tNiemanas
	end
--]]
	
	if (devicechanged['Testowy']) then
		print('---------- Wciśnięto Testowy ------------')
		--print(otherdevices_svalues['Salon'])
		--if oknoZamkniete('Okno Salon') then print('TRUE')
		--else print('FALSE')
		--end
		zmien, json = termostatSetPoint('Salon', 13, czasSalon[1], tNoc, czasSalon[2], tDzien, czasSalon[3], tNoc, 'Okno Salon', 6, wyl)
		if zmien then commandArray[1]={['OpenURL']=json} end
		
		zmien, json = termostatSetPoint('Kuchnia', 14, czasKuchnia[1], tNoc, czasKuchnia[2], tDzien, czasKuchnia[3], tNoc, 'Okno Kuchnia', 0, wyl)
		if zmien then commandArray[2]={['OpenURL']=json} end
		
		zmien, json = termostatSetPoint('Bąble', 8, czasBable[1], tNoc, czasBable[2], tDzien, czasBable[3], tNocBable, 'Okno Bąble', 0, wyl)
		if zmien then commandArray[3]={['OpenURL']=json} end
		
		zmien, json = termostatSetPoint('Hubert', 15, czasHubert[1], tNoc, czasHubert[2], tDzien, czasHubert[3], tNoc, 'Okno Hubert', 0, wyl)
		if zmien then commandArray[4]={['OpenURL']=json} end
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
