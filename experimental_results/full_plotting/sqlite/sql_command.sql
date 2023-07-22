/* For improvement plot */
/* Obtain improvement of geometric means of the objective grouped by algorithm and k */
SELECT	a.algorithm, 
	a.k, 
	(100*(c.obj_func/a.obj_func - 1)) AS improvement  
from (	SELECT	b.algorithm, 
		b.k, 
		EXP(SUM(LN(b.obj_func))/COUNT(*)) AS obj_func  
	FROM  base_GP b  
	GROUP BY b.algorithm, b.k) a 
JOIN (	SELECT	b.algorithm, 
		b.k, 
		EXP(SUM(LN(b.obj_func))/COUNT(*)) AS obj_func 
	FROM base_GP b 
	WHERE b.algorithm="int_OrigAlph_RecMSecfennelapprosqrt4R1" 
	GROUP BY b.k) c 
ON	a.k = c.k 
ORDER BY a.algorithm, a.k ASC;


/* Obtain performance profile for each algorithm  */
SELECT	a.algorithm, 
	(a.obj_func/c.obj_func) AS ratio_to_best 
FROM base_GP a 
JOIN (	SELECT b.graph, 
		b.seed, 
		b.k, 
		MIN(b.obj_func) AS obj_func 
	FROM base_GP b 
	GROUP BY b.graph, 
		b.seed, 
		b.k) c  
ON	a.k = c.k AND 
	a.graph = c.graph AND 
	a.seed = c.seed 
ORDER BY a.algorithm, ratio_to_best ASC;


/* For running time ratio plot */
/* Obtain ratio to highest out of geometric means of the objective grouped by algorithm and k */
SELECT	a.algorithm, 
	a.k, 
	(a.obj_func/c.obj_func) AS ratio  
from (	SELECT	b.algorithm, 
		b.k, 
		EXP(SUM(LN(b.obj_func))/COUNT(*)) AS obj_func  
	FROM  base_GP b  
	GROUP BY b.algorithm, b.k) a 
JOIN (	SELECT	x.k, 
		MAX(x.obj_func) AS obj_func
	FROM (	SELECT	b.algorithm, 
			b.k, 
			EXP(SUM(LN(b.obj_func))/COUNT(*)) AS obj_func 
		FROM base_GP b 
		GROUP BY b.algorithm,
			b.k ) x  
	GROUP BY x.k) c 
ON	a.k = c.k 
ORDER BY a.algorithm, a.k ASC;

