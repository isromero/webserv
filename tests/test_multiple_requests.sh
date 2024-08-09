URL="http://localhost:6969"
NUM_REQUESTS=10000

for i in $(seq 1 $NUM_REQUESTS); do
  curl -s -o /dev/null $URL
done
