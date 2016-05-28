rate=0.001
error=3

for i in `seq 1 5`
do
  ./waf --run "dumbbell-animation --nLeaf=1 --transport_prot=TcpWestwoodPlus --error=$rate --pcapFile=dumbbell-westwoodplussack-bursterror-$error --sack=yes"
  ./waf --run "dumbbell-animation --nLeaf=1 --transport_prot=TcpWestwoodPlus --error=$rate --pcapFile=dumbbell-westwoodplus-bursterror-$error"
  ./waf --run "dumbbell-animation --nLeaf=1 --transport_prot=TcpNewReno --error=$rate --pcapFile=dumbbell-newrenosack-bursterror-$error --sack=yes"
  ./waf --run "dumbbell-animation --nLeaf=1 --transport_prot=TcpNewReno --error=$rate --pcapFile=dumbbell-newreno-bursterror-$error"
  ./waf --run "dumbbell-animation --nLeaf=1 --transport_prot=TcpWestwood --error=$rate --pcapFile=dumbbell-westwood-bursterror-$error"
  ./waf --run "dumbbell-animation --nLeaf=1 --transport_prot=TcpWestwood --error=$rate --pcapFile=dumbbell-westwoodsack-bursterror-$error --sack=yes"

  rate=`echo "$rate / 10" | bc -l`
  ((error++))
done

rm -rf dumbbell/dumbbell-{westwoodplussack,westwoodplus,newreno,newrenosack,westwood,westwoodsack,sack}-bursterror-3-0-1.pcap dumbbell/dumbbell-{westwoodplussack,westwoodplus,newreno,newrenosack,westwood,westwoodsack}-bursterror-3-{1..3}-* dumbbell/dumbbell-{westwoodplussack,westwoodplus,newreno,newrenosack,westwood,westwoodsack}-bursterror-4-0-1.pcap dumbbell/dumbbell-{westwoodplussack,westwoodplus,newreno,newrenosack,westwood,westwoodsack}-bursterror-4-{1..3}-* dumbbell/dumbbell-{westwoodplussack,westwoodplus,newreno,newrenosack,westwood,westwoodsack}-bursterror-5-0-1.pcap dumbbell/dumbbell-{westwoodplussack,westwoodplus,newreno,newrenosack,westwood,westwoodsack}-bursterror-5-{1..3}-* dumbbell/dumbbell-{westwoodplussack,westwoodplus,newreno,newrenosack,westwood,westwoodsack}-bursterror-6-0-1.pcap dumbbell/dumbbell-{westwoodplussack,westwoodplus,newreno,newrenosack,westwood,westwoodsack}-bursterror-6-{1..3}-* dumbbell/dumbbell-{westwoodplussack,westwoodplus,newreno,newrenosack,westwood,westwoodsack}-bursterror-7-0-1.pcap dumbbell/dumbbell-{westwoodplussack,westwoodplus,newreno,newrenosack,westwood,westwoodsack}-bursterror-7-{1..3}-*


