CREATE OR REPLACE FUNCTION generic0(input_array real []) RETURNS real
AS
$BODY$
	declare compare_value real := -1.0;
	declare compare_index int;
	declare leftchild int := -1;
	declare rightchild int := -1;
	declare node_id int := 0;
	declare cname text;
	declare isleaf boolean = FALSE;
begin
	LOOP
		select tree0.index into compare_index from tree0 where id = node_id;
		select tree0.value into compare_value from tree0 where id = node_id;
		select tree0.isleaf into isleaf from tree0 where id = node_id;
		select tree0.leftchild into leftchild from tree0 where id = node_id;
		select tree0.rightchild into rightchild from tree0 where id = node_id;

		EXIT WHEN isleaf = TRUE;
		
		if(input_array[compare_index] <= compare_value)
		then node_id := leftchild;
		else node_id := rightchild;
		end if;

	END LOOP;
	return compare_value;
end;
$BODY$
LANGUAGE plpgsql VOLATILE
COST 100;

CREATE OR REPLACE FUNCTION generic1(input_array real []) RETURNS real
AS
$BODY$
	declare compare_value real := -1.0;
	declare compare_index int;
	declare leftchild int := -1;
	declare rightchild int := -1;
	declare node_id int := 0;
	declare cname text;
	declare isleaf boolean = FALSE;
begin
	LOOP
		select tree1.index into compare_index from tree1 where id = node_id;
		select tree1.value into compare_value from tree1 where id = node_id;
		select tree1.isleaf into isleaf from tree1 where id = node_id;
		select tree1.leftchild into leftchild from tree1 where id = node_id;
		select tree1.rightchild into rightchild from tree1 where id = node_id;

		EXIT WHEN isleaf = TRUE;
		
		if(input_array[compare_index] <= compare_value)
		then node_id := leftchild;
		else node_id := rightchild;
		end if;

	END LOOP;
	return compare_value;
end;
$BODY$
LANGUAGE plpgsql VOLATILE
COST 100;

CREATE OR REPLACE FUNCTION generic2(input_array real []) RETURNS real
AS
$BODY$
	declare compare_value real := -1.0;
	declare compare_index int;
	declare leftchild int := -1;
	declare rightchild int := -1;
	declare node_id int := 0;
	declare cname text;
	declare isleaf boolean = FALSE;
begin
	LOOP
		select tree2.index into compare_index from tree2 where id = node_id;
		select tree2.value into compare_value from tree2 where id = node_id;
		select tree2.isleaf into isleaf from tree2 where id = node_id;
		select tree2.leftchild into leftchild from tree2 where id = node_id;
		select tree2.rightchild into rightchild from tree2 where id = node_id;

		EXIT WHEN isleaf = TRUE;
		
		if(input_array[compare_index] <= compare_value)
		then node_id := leftchild;
		else node_id := rightchild;
		end if;

	END LOOP;
	return compare_value;
end;
$BODY$
LANGUAGE plpgsql VOLATILE
COST 100;

CREATE OR REPLACE FUNCTION generic3(input_array real []) RETURNS real
AS
$BODY$
	declare compare_value real := -1.0;
	declare compare_index int;
	declare leftchild int := -1;
	declare rightchild int := -1;
	declare node_id int := 0;
	declare cname text;
	declare isleaf boolean = FALSE;
begin
	LOOP
		select tree3.index into compare_index from tree3 where id = node_id;
		select tree3.value into compare_value from tree3 where id = node_id;
		select tree3.isleaf into isleaf from tree3 where id = node_id;
		select tree3.leftchild into leftchild from tree3 where id = node_id;
		select tree3.rightchild into rightchild from tree3 where id = node_id;

		EXIT WHEN isleaf = TRUE;
		
		if(input_array[compare_index] <= compare_value)
		then node_id := leftchild;
		else node_id := rightchild;
		end if;

	END LOOP;
	return compare_value;
end;
$BODY$
LANGUAGE plpgsql VOLATILE
COST 100;

CREATE OR REPLACE FUNCTION generic4(input_array real []) RETURNS real
AS
$BODY$
	declare compare_value real := -1.0;
	declare compare_index int;
	declare leftchild int := -1;
	declare rightchild int := -1;
	declare node_id int := 0;
	declare cname text;
	declare isleaf boolean = FALSE;
begin
	LOOP
		select tree4.index into compare_index from tree4 where id = node_id;
		select tree4.value into compare_value from tree4 where id = node_id;
		select tree4.isleaf into isleaf from tree4 where id = node_id;
		select tree4.leftchild into leftchild from tree4 where id = node_id;
		select tree4.rightchild into rightchild from tree4 where id = node_id;

		EXIT WHEN isleaf = TRUE;
		
		if(input_array[compare_index] <= compare_value)
		then node_id := leftchild;
		else node_id := rightchild;
		end if;

	END LOOP;
	return compare_value;
end;
$BODY$
LANGUAGE plpgsql VOLATILE
COST 100;

CREATE OR REPLACE FUNCTION generic5(input_array real []) RETURNS real
AS
$BODY$
	declare compare_value real := -1.0;
	declare compare_index int;
	declare leftchild int := -1;
	declare rightchild int := -1;
	declare node_id int := 0;
	declare cname text;
	declare isleaf boolean = FALSE;
begin
	LOOP
		select tree5.index into compare_index from tree5 where id = node_id;
		select tree5.value into compare_value from tree5 where id = node_id;
		select tree5.isleaf into isleaf from tree5 where id = node_id;
		select tree5.leftchild into leftchild from tree5 where id = node_id;
		select tree5.rightchild into rightchild from tree5 where id = node_id;

		EXIT WHEN isleaf = TRUE;
		
		if(input_array[compare_index] <= compare_value)
		then node_id := leftchild;
		else node_id := rightchild;
		end if;

	END LOOP;
	return compare_value;
end;
$BODY$
LANGUAGE plpgsql VOLATILE
COST 100;

CREATE OR REPLACE FUNCTION generic6(input_array real []) RETURNS real
AS
$BODY$
	declare compare_value real := -1.0;
	declare compare_index int;
	declare leftchild int := -1;
	declare rightchild int := -1;
	declare node_id int := 0;
	declare cname text;
	declare isleaf boolean = FALSE;
begin
	LOOP
		select tree6.index into compare_index from tree6 where id = node_id;
		select tree6.value into compare_value from tree6 where id = node_id;
		select tree6.isleaf into isleaf from tree6 where id = node_id;
		select tree6.leftchild into leftchild from tree6 where id = node_id;
		select tree6.rightchild into rightchild from tree6 where id = node_id;

		EXIT WHEN isleaf = TRUE;
		
		if(input_array[compare_index] <= compare_value)
		then node_id := leftchild;
		else node_id := rightchild;
		end if;

	END LOOP;
	return compare_value;
end;
$BODY$
LANGUAGE plpgsql VOLATILE
COST 100;

CREATE OR REPLACE FUNCTION generic7(input_array real []) RETURNS real
AS
$BODY$
	declare compare_value real := -1.0;
	declare compare_index int;
	declare leftchild int := -1;
	declare rightchild int := -1;
	declare node_id int := 0;
	declare cname text;
	declare isleaf boolean = FALSE;
begin
	LOOP
		select tree7.index into compare_index from tree7 where id = node_id;
		select tree7.value into compare_value from tree7 where id = node_id;
		select tree7.isleaf into isleaf from tree7 where id = node_id;
		select tree7.leftchild into leftchild from tree7 where id = node_id;
		select tree7.rightchild into rightchild from tree7 where id = node_id;

		EXIT WHEN isleaf = TRUE;
		
		if(input_array[compare_index] <= compare_value)
		then node_id := leftchild;
		else node_id := rightchild;
		end if;

	END LOOP;
	return compare_value;
end;
$BODY$
LANGUAGE plpgsql VOLATILE
COST 100;

CREATE OR REPLACE FUNCTION generic8(input_array real []) RETURNS real
AS
$BODY$
	declare compare_value real := -1.0;
	declare compare_index int;
	declare leftchild int := -1;
	declare rightchild int := -1;
	declare node_id int := 0;
	declare cname text;
	declare isleaf boolean = FALSE;
begin
	LOOP
		select tree8.index into compare_index from tree8 where id = node_id;
		select tree8.value into compare_value from tree8 where id = node_id;
		select tree8.isleaf into isleaf from tree8 where id = node_id;
		select tree8.leftchild into leftchild from tree8 where id = node_id;
		select tree8.rightchild into rightchild from tree8 where id = node_id;

		EXIT WHEN isleaf = TRUE;
		
		if(input_array[compare_index] <= compare_value)
		then node_id := leftchild;
		else node_id := rightchild;
		end if;

	END LOOP;
	return compare_value;
end;
$BODY$
LANGUAGE plpgsql VOLATILE
COST 100;

CREATE OR REPLACE FUNCTION generic9(input_array real []) RETURNS real
AS
$BODY$
	declare compare_value real := -1.0;
	declare compare_index int;
	declare leftchild int := -1;
	declare rightchild int := -1;
	declare node_id int := 0;
	declare cname text;
	declare isleaf boolean = FALSE;
begin
	LOOP
		select tree9.index into compare_index from tree9 where id = node_id;
		select tree9.value into compare_value from tree9 where id = node_id;
		select tree9.isleaf into isleaf from tree9 where id = node_id;
		select tree9.leftchild into leftchild from tree9 where id = node_id;
		select tree9.rightchild into rightchild from tree9 where id = node_id;

		EXIT WHEN isleaf = TRUE;
		
		if(input_array[compare_index] <= compare_value)
		then node_id := leftchild;
		else node_id := rightchild;
		end if;

	END LOOP;
	return compare_value;
end;
$BODY$
LANGUAGE plpgsql VOLATILE
COST 100;
