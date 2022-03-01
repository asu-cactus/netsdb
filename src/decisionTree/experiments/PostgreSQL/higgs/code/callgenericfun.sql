CREATE OR REPLACE FUNCTION call_generic() RETURNS table (result real) 
AS
$BODY$
	declare single_result real[10];
	declare single_value real := -1;

	declare leptonpT real;
	declare leptoneta real;
	declare leptonphi real;
	declare missingenergymagnitude real;
	declare missingenergyphi real;
	declare jet1pt real;
	declare jet1eta real;
	declare jet1phi real;
	declare jet1btag real;
	declare jet2pt real;
	declare jet2eta real;
	declare jet2phi real;
	declare jet2btag real;
	declare jet3pt real;
	declare jet3eta real;
	declare jet3phi real;
	declare jet3btag real;
	declare jet4pt real;
	declare jet4eta real;
	declare jet4phi real;
	declare jet4btag real;
	declare mjj real;
	declare mjjj real;
	declare mlv real;
	declare mjlv real;
	declare mbb real;
	declare mwbb real;
	declare mwwbb real;
	
	declare countnum int[10];
	declare thiscount int := 0;
	declare rec record;
	declare most int := 0;
	declare mostvalue real;
	declare mostposition int := 0;
	declare single_array real[28];
	declare cur_records scroll cursor for SELECT * from higgs;

begin
	for i in 1..1
	LOOP
		open cur_records;
		LOOP
			fetch cur_records into rec;
			exit when not found;

			single_array[0] := rec.leptonpT;
			single_array[1] := rec.leptoneta;
			single_array[2] := rec.leptonphi;
			single_array[3] := rec.missingenergymagnitude;
			single_array[4] := rec.missingenergyphi;
			single_array[5] := rec.jet1pt;
			single_array[6] := rec.jet1eta;
			single_array[7] := rec.jet1phi;
			single_array[8] := rec.jet1btag;
			single_array[9] := rec.jet2pt;
			single_array[10] := rec.jet2eta;
			single_array[11] := rec.jet2phi;
			single_array[12] := rec.jet2btag;
			single_array[13] := rec.jet3pt;
			single_array[14] := rec.jet3eta;
			single_array[15] := rec.jet3phi;
			single_array[16] := rec.jet3btag;
			single_array[17] := rec.jet4pt;
			single_array[18] := rec.jet4eta;
			single_array[19] := rec.jet4phi;
			single_array[20] := rec.jet4btag;
			single_array[21] := rec.mjj;
			single_array[22] := rec.mjjj;
			single_array[23] := rec.mlv;
			single_array[24] := rec.mjlv;
			single_array[25] := rec.mbb;
			single_array[26] := rec.mwbb;
			single_array[27] := rec.mwwbb;

			EXECUTE format('SELECT generic0($1);') using single_array into single_value;
			single_result[0] := single_value;
			EXECUTE format('SELECT generic1($1);') using single_array into single_value;
			single_result[1] := single_value;
			EXECUTE format('SELECT generic2($1);') using single_array into single_value;
			single_result[2] := single_value;
			EXECUTE format('SELECT generic3($1);') using single_array into single_value;
			single_result[3] := single_value;
			EXECUTE format('SELECT generic4($1);') using single_array into single_value;
			single_result[4] := single_value;
			EXECUTE format('SELECT generic5($1);') using single_array into single_value;
			single_result[5] := single_value;
			EXECUTE format('SELECT generic6($1);') using single_array into single_value;
			single_result[6] := single_value;
			EXECUTE format('SELECT generic7($1);') using single_array into single_value;
			single_result[7] := single_value;
			EXECUTE format('SELECT generic8($1);') using single_array into single_value;
			single_result[8] := single_value;
			EXECUTE format('SELECT generic9($1);') using single_array into single_value;
			single_result[9] := single_value;
			
			for j in 1..10
			LOOP
				for k in 1..10
				LOOP
					if(single_result[j] = single_result[k])
					then thiscount := thiscount + 1;
					END IF;
				END LOOP;
				countnum[j] := thiscount;
				thiscount := 0;
			END LOOP;
			EXECUTE format('SELECT max(x) from unnest($1) x;') using countnum into most;
			EXECUTE format('SELECT array_position($1,$2);') using countnum, most into mostposition;
			result := single_result[mostposition];
			return next;
		END LOOP;
		close cur_records;
	END LOOP;
end;
$BODY$
LANGUAGE plpgsql VOLATILE
COST 100;
