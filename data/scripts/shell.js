function shell(pipeline, shell, errors){
	if ((shell == undefined) || typeof(shell) == void 0)
		shell = "bash"
	// quote ipipeline and escape inside quotes
	quotedPipeline = "\"" + pipeline.replace(/"/g, "\\\"") + "\"";
	command = shell + " -c " + quotedPipeline;
//	printOrange(command, "\n");
	outcome = execute(command, null, errors);
	return outcome;
	}
