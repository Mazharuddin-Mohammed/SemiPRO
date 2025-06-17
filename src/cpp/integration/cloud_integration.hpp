// Author: Dr. Mazharuddin Mohammed
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <future>
#include <functional>

namespace SemiPRO {

/**
 * @brief Cloud provider types
 */
enum class CloudProvider {
    AWS,
    AZURE,
    GCP,
    ALIBABA_CLOUD,
    PRIVATE_CLOUD
};

/**
 * @brief Container configuration
 */
struct ContainerConfig {
    std::string image_name;
    std::string tag = "latest";
    std::unordered_map<std::string, std::string> environment_vars;
    std::unordered_map<std::string, std::string> volume_mounts;
    std::vector<std::string> command_args;
    int cpu_cores = 2;
    int memory_gb = 4;
    int gpu_count = 0;
    std::string gpu_type = "nvidia-tesla-v100";
};

/**
 * @brief Cluster configuration
 */
struct ClusterConfig {
    std::string cluster_name;
    CloudProvider provider;
    std::string region;
    std::string instance_type;
    int min_nodes = 1;
    int max_nodes = 10;
    int desired_nodes = 2;
    bool auto_scaling = true;
    std::unordered_map<std::string, std::string> node_labels;
    std::vector<std::string> availability_zones;
};

/**
 * @brief Job configuration for cloud execution
 */
struct CloudJob {
    std::string job_id;
    std::string job_name;
    ContainerConfig container;
    std::unordered_map<std::string, std::string> input_files;
    std::vector<std::string> output_files;
    int timeout_minutes = 60;
    int retry_count = 3;
    std::string queue_name = "default";
    std::unordered_map<std::string, std::string> metadata;
};

/**
 * @brief Job status and result
 */
struct JobResult {
    enum Status { PENDING, RUNNING, COMPLETED, FAILED, CANCELLED, TIMEOUT };
    Status status;
    std::string job_id;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    std::unordered_map<std::string, std::string> output_data;
    std::vector<std::string> log_messages;
    std::string error_message;
    double cost_estimate = 0.0;
};

/**
 * @brief Cloud storage configuration
 */
struct StorageConfig {
    CloudProvider provider;
    std::string bucket_name;
    std::string region;
    std::string access_key;
    std::string secret_key;
    std::string endpoint_url;
    bool use_ssl = true;
};

/**
 * @brief Main cloud integration class
 */
class CloudIntegration {
private:
    CloudProvider provider_;
    std::string credentials_file_;
    std::unordered_map<std::string, std::string> config_;
    std::unordered_map<std::string, JobResult> job_results_;
    
public:
    CloudIntegration(CloudProvider provider, const std::string& credentials_file = "");
    ~CloudIntegration();
    
    // Cluster management
    std::string createCluster(const ClusterConfig& config);
    bool deleteCluster(const std::string& cluster_id);
    std::vector<std::string> listClusters();
    ClusterConfig getClusterInfo(const std::string& cluster_id);
    bool scaleCluster(const std::string& cluster_id, int desired_nodes);
    
    // Job management
    std::string submitJob(const CloudJob& job);
    std::string submitBatchJobs(const std::vector<CloudJob>& jobs);
    JobResult getJobStatus(const std::string& job_id);
    bool cancelJob(const std::string& job_id);
    std::vector<JobResult> listJobs(const std::string& queue_name = "");
    
    // Container management
    std::string buildContainer(const std::string& dockerfile_path, const std::string& image_name);
    bool pushContainer(const std::string& image_name, const std::string& registry_url = "");
    bool pullContainer(const std::string& image_name);
    std::vector<std::string> listContainers();
    
    // Storage operations
    bool uploadFile(const std::string& local_path, const std::string& remote_path, 
                   const StorageConfig& storage);
    bool downloadFile(const std::string& remote_path, const std::string& local_path,
                     const StorageConfig& storage);
    bool deleteFile(const std::string& remote_path, const StorageConfig& storage);
    std::vector<std::string> listFiles(const std::string& remote_prefix, const StorageConfig& storage);
    
    // Monitoring and logging
    std::vector<std::string> getJobLogs(const std::string& job_id);
    std::unordered_map<std::string, double> getClusterMetrics(const std::string& cluster_id);
    double estimateJobCost(const CloudJob& job);
    std::unordered_map<std::string, double> getBillingInfo(const std::string& time_period);
    
    // Auto-scaling and optimization
    void enableAutoScaling(const std::string& cluster_id, int min_nodes, int max_nodes);
    void setSpotInstances(const std::string& cluster_id, bool enable, double max_price = 0.0);
    std::vector<std::string> optimizeInstanceTypes(const std::vector<CloudJob>& jobs);
    
    // Security and compliance
    void enableEncryption(const std::string& cluster_id, const std::string& key_id = "");
    void setNetworkPolicy(const std::string& cluster_id, const std::string& policy);
    bool auditClusterSecurity(const std::string& cluster_id);
    
private:
    void initializeProvider();
    std::string generateJobId();
    void updateJobStatus(const std::string& job_id, JobResult::Status status);
    
    // Provider-specific implementations
    std::string createAWSCluster(const ClusterConfig& config);
    std::string createAzureCluster(const ClusterConfig& config);
    std::string createGCPCluster(const ClusterConfig& config);
    
    std::string submitAWSJob(const CloudJob& job);
    std::string submitAzureJob(const CloudJob& job);
    std::string submitGCPJob(const CloudJob& job);
};

/**
 * @brief AWS-specific integration
 */
class AWSIntegration {
private:
    std::string access_key_;
    std::string secret_key_;
    std::string region_;
    
public:
    AWSIntegration(const std::string& access_key, const std::string& secret_key, 
                  const std::string& region = "us-east-1");
    
    // EKS cluster management
    std::string createEKSCluster(const ClusterConfig& config);
    bool deleteEKSCluster(const std::string& cluster_name);
    
    // Batch job management
    std::string submitBatchJob(const CloudJob& job);
    JobResult getBatchJobStatus(const std::string& job_id);
    
    // S3 storage operations
    bool uploadToS3(const std::string& bucket, const std::string& key, const std::string& file_path);
    bool downloadFromS3(const std::string& bucket, const std::string& key, const std::string& file_path);
    
    // EC2 instance management
    std::vector<std::string> launchEC2Instances(const std::string& ami_id, const std::string& instance_type, int count);
    bool terminateEC2Instance(const std::string& instance_id);
    
    // Lambda function integration
    std::string createLambdaFunction(const std::string& function_name, const std::string& code_path);
    std::string invokeLambdaFunction(const std::string& function_name, const std::string& payload);
    
private:
    void authenticateAWS();
    std::string executeAWSCommand(const std::string& command);
};

/**
 * @brief Azure-specific integration
 */
class AzureIntegration {
private:
    std::string subscription_id_;
    std::string tenant_id_;
    std::string client_id_;
    std::string client_secret_;
    
public:
    AzureIntegration(const std::string& subscription_id, const std::string& tenant_id,
                    const std::string& client_id, const std::string& client_secret);
    
    // AKS cluster management
    std::string createAKSCluster(const ClusterConfig& config);
    bool deleteAKSCluster(const std::string& cluster_name);
    
    // Azure Batch integration
    std::string submitAzureBatchJob(const CloudJob& job);
    JobResult getAzureBatchJobStatus(const std::string& job_id);
    
    // Azure Storage operations
    bool uploadToBlob(const std::string& container, const std::string& blob_name, const std::string& file_path);
    bool downloadFromBlob(const std::string& container, const std::string& blob_name, const std::string& file_path);
    
    // Virtual Machine management
    std::vector<std::string> createVMs(const std::string& vm_size, int count);
    bool deleteVM(const std::string& vm_id);
    
private:
    void authenticateAzure();
    std::string executeAzureCommand(const std::string& command);
};

/**
 * @brief Google Cloud Platform integration
 */
class GCPIntegration {
private:
    std::string project_id_;
    std::string service_account_key_;
    std::string region_;
    
public:
    GCPIntegration(const std::string& project_id, const std::string& service_account_key,
                  const std::string& region = "us-central1");
    
    // GKE cluster management
    std::string createGKECluster(const ClusterConfig& config);
    bool deleteGKECluster(const std::string& cluster_name);
    
    // Cloud Run integration
    std::string deployCloudRun(const std::string& service_name, const std::string& image_url);
    std::string invokeCloudRun(const std::string& service_url, const std::string& payload);
    
    // Cloud Storage operations
    bool uploadToGCS(const std::string& bucket, const std::string& object_name, const std::string& file_path);
    bool downloadFromGCS(const std::string& bucket, const std::string& object_name, const std::string& file_path);
    
    // Compute Engine management
    std::vector<std::string> createInstances(const std::string& machine_type, int count);
    bool deleteInstance(const std::string& instance_id);
    
    // BigQuery integration
    std::string createBigQueryDataset(const std::string& dataset_name);
    bool uploadToBigQuery(const std::string& dataset, const std::string& table, const std::string& data_file);
    
private:
    void authenticateGCP();
    std::string executeGCloudCommand(const std::string& command);
};

/**
 * @brief Docker container utilities
 */
class DockerUtils {
public:
    static std::string buildImage(const std::string& dockerfile_path, const std::string& image_name);
    static bool pushImage(const std::string& image_name, const std::string& registry_url);
    static bool pullImage(const std::string& image_name);
    static std::string runContainer(const ContainerConfig& config);
    static bool stopContainer(const std::string& container_id);
    static std::vector<std::string> listContainers();
    static std::string getContainerLogs(const std::string& container_id);
    
    // Kubernetes utilities
    static std::string deployToKubernetes(const std::string& yaml_file);
    static bool deleteKubernetesDeployment(const std::string& deployment_name);
    static std::vector<std::string> getKubernetesPods(const std::string& namespace = "default");
    static std::string getKubernetesPodLogs(const std::string& pod_name);
    
private:
    static std::string executeDockerCommand(const std::string& command);
    static std::string executeKubectlCommand(const std::string& command);
};

} // namespace SemiPRO
