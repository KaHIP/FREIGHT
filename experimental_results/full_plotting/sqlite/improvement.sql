/* For improvement plot */
/* Obtain improvement of geometric means of the objective grouped by algorithm and k */
SELECT	k AS ind,
	improvement AS "generic_label"
FROM	(
	SELECT	CASE WHEN a.geomean_generic_objective = 0 THEN
			CASE WHEN c.geomean_generic_objective = 0 THEN
				0.0 
			ELSE
				2000.0
			END
		ELSE
			(100*(c.geomean_generic_objective/a.geomean_generic_objective - 1))
		END AS improvement,
		a.k
	FROM (	SELECT	d.k,
		ROUND(EXP(AVG(LN(d.generic_objective))),3) AS geomean_generic_objective
		FROM (	SELECT	b.graph,
				b.k, 
				AVG(b.generic_objective) AS generic_objective  
			FROM  generic_table b  
			WHERE b.algorithm="generic_algorithm" 
			GROUP BY b.graph, b.k ) d 
		GROUP BY d.k ) a 
	JOIN (	SELECT d.k,
		ROUND(EXP(AVG(LN(d.generic_objective))),3) AS geomean_generic_objective
		FROM ( SELECT	b.graph,
				b.k, 
				AVG(b.generic_objective) generic_objective 
			FROM generic_table b 
			WHERE b.algorithm="reference_algorithm" 
			GROUP BY b.graph, b.k ) d
		GROUP BY d.k ) c 
	ON	a.k = c.k
	);
