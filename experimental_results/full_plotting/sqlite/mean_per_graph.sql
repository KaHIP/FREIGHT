/* Obtain averages per algorithm and graph */
SELECT	a.graph,
	a.algorithm, 
	ROUND(AVG(generic_objective),3) AS avg_generic_objective,
	ROUND(EXP(AVG(LN(generic_objective))),3) AS geomean_generic_objective, 
	ROUND(MAX(generic_objective),3) AS max_generic_objective,
	ROUND(MIN(generic_objective),3) AS min_generic_objective
FROM generic_table a
GROUP BY a.graph, a.algorithm;
