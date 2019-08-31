/**
	Additional functions for proplists.
 */

/**
 * Creates a proplist in the object or in a given proplist.
 * Creation is omitted if such a property already exists
 * in the parent.
 *
 * @par name The name of the proplist as a property of parent
 * @par parent (optional) Creates the proplist in this proplist.
 *             Defaults to {@code this}, i.e. the context that
 *             calls this function.
 * @return proplist the newly created proplist.
 */
global func CreateProplist(string name, proplist parent)
{
	parent = parent ?? this;
	AssertNotNil(parent);

	if (!parent[name])
	{
		parent[name] = {};
	}
	return parent[name];
}
