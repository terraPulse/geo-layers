echo "Cleaning up old images"
docker rmi -f 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l0
docker rmi -f 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l0.1
docker rmi -f 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l1
docker rmi -f 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l1.1
docker rmi -f 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l1.2
docker rmi -f 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l2
docker rmi -f 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l2.1
docker image prune

echo "Building Layer l0 ..."
docker build -t 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l0 ./l0
docker push 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l0

echo "Building Layer l0.1 ..."
rm -rf l0.1/bin
cp -rf bin l0.1/bin
docker build -t 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l0.1 ./l0.1
docker push 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l0.1

echo "Building Layer l1 ..."
docker build -t 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l1 ./l1
docker push 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l1

echo "Building Layer l1.1 ..."
rm -rf l1.1/bin
cp -rf bin l1.1/bin
docker build -t 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l1.1 ./l1.1
docker push 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l1.1

echo "Building Layer l1.2 ..."
rm -rf l1.2/bin
cp -rf bin l1.2/bin
docker build -t 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l1.2 ./l1.2
docker push 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l1.2

echo "Building Layer l2 ..."
docker build -t 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l2 ./l2
docker push 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l2

echo "Building Layer l2.1 ..."
rm -rf l2.1/bin
cp -rf bin l2.1/bin
docker build -t 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l2.1 ./l2.1
docker push 544020760996.dkr.ecr.us-east-1.amazonaws.com/geo-layers:l2.1
