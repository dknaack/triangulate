# Simple polygon triangulation library

`triangulate` is a single header polygon triangulation library. It only
accepts simple polygon with no holes and since it uses the index buffer for
triangulation, it allocates zero memory.

## Example

See `example.c`. The example generates a random simple polygon and triangulates
it using this library. You can compile it with `make`:

	make
	./example
